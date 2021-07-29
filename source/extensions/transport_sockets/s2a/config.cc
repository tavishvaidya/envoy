#include "source/extensions/transport_sockets/s2a/config.h"

#include "envoy/extensions/transport_sockets/s2a/v3alpha/s2a.pb.h"
#include "envoy/extensions/transport_sockets/s2a/v3alpha/s2a.pb.validate.h"
#include "envoy/registry/registry.h"
#include "envoy/server/transport_socket_config.h"

#include "source/common/common/assert.h"
#include "source/common/grpc/google_grpc_context.h"
#include "source/common/protobuf/protobuf.h"
#include "source/common/protobuf/utility.h"
#include "source/extensions/transport_sockets/s2a/grpc_tsi.h"
#include "source/extensions/transport_sockets/s2a/tsi_socket.h"

#include "absl/container/node_hash_set.h"
#include "absl/strings/str_join.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace S2A {

// smart pointer for grpc_alts_credentials_options that will be automatically freed.
using GrpcAltsCredentialsOptionsPtr =
    CSmartPtr<grpc_alts_credentials_options, grpc_alts_credentials_options_destroy>;

namespace {

// TODO: gRPC v1.30.0-pre1 defines the equivalent function grpc_alts_set_rpc_protocol_versions
// that should be called directly when available.
void grpcAltsSetRpcProtocolVersions(grpc_gcp_rpc_protocol_versions* rpc_versions) {
  grpc_gcp_rpc_protocol_versions_set_max(rpc_versions, GRPC_PROTOCOL_VERSION_MAX_MAJOR,
                                         GRPC_PROTOCOL_VERSION_MAX_MINOR);
  grpc_gcp_rpc_protocol_versions_set_min(rpc_versions, GRPC_PROTOCOL_VERSION_MIN_MAJOR,
                                         GRPC_PROTOCOL_VERSION_MIN_MINOR);
}

// Manage ALTS singleton state via SingletonManager
class AltsSharedState : public Singleton::Instance {
public:
  AltsSharedState() { grpc_alts_shared_resource_dedicated_init(); }

  ~AltsSharedState() override { grpc_alts_shared_resource_dedicated_shutdown(); }

private:
  // There is blanket google-grpc initialization in MainCommonBase, but that
  // doesn't cover unit tests. However, putting blanket coverage in ProcessWide
  // causes background threaded memory allocation in all unit tests making it
  // hard to measure memory. Thus we also initialize grpc using our idempotent
  // wrapper-class in classes that need it. See
  // https://github.com/envoyproxy/envoy/issues/8282 for details.
#ifdef ENVOY_GOOGLE_GRPC
  Grpc::GoogleGrpcContext google_grpc_context_;
#endif
};

SINGLETON_MANAGER_REGISTRATION(alts_shared_state);

Network::TransportSocketFactoryPtr createTransportSocketFactoryHelper(
    const Protobuf::Message& message, bool is_upstream,
    Server::Configuration::TransportSocketFactoryContext& factory_ctxt) {
  auto alts_shared_state = factory_ctxt.singletonManager().getTyped<AltsSharedState>(
      SINGLETON_MANAGER_REGISTERED_NAME(alts_shared_state),
      [] { return std::make_shared<AltsSharedState>(); });      
  auto config =
      MessageUtil::downcastAndValidate<const envoy::extensions::transport_sockets::s2a::v3alpha::S2AConfiguration&>(
          message, factory_ctxt.messageValidationVisitor());

  const std::string& handshaker_service = config.s2a_address();
  HandshakerFactory factory =
      [handshaker_service, is_upstream,
       alts_shared_state](Event::Dispatcher& dispatcher,
                          const Network::Address::InstanceConstSharedPtr& local_address,
                          const Network::Address::InstanceConstSharedPtr&) -> TsiHandshakerPtr {
    ASSERT(local_address != nullptr);

    GrpcAltsCredentialsOptionsPtr options;
    if (is_upstream) {
      options = GrpcAltsCredentialsOptionsPtr(grpc_alts_credentials_client_options_create());
    } else {
      options = GrpcAltsCredentialsOptionsPtr(grpc_alts_credentials_server_options_create());
    }
    grpcAltsSetRpcProtocolVersions(&options->rpc_versions);
    const char* target_name = is_upstream ? "" : nullptr;
    tsi_handshaker* handshaker = nullptr;
    // Specifying target name as empty since TSI won't take care of validating peer identity
    // in this use case. The validation will be performed by TsiSocket with the validator.
    // Set the max frame size to 16KB.
    tsi_result status = alts_tsi_handshaker_create(
        options.get(), target_name, handshaker_service.c_str(), is_upstream,
        nullptr /* interested_parties */, &handshaker, 16384 /* default max frame size */);
    CHandshakerPtr handshaker_ptr{handshaker};

    if (status != TSI_OK) {
      const std::string handshaker_name = is_upstream ? "client" : "server";
      ENVOY_LOG_MISC(warn, "Cannot create ATLS {} handshaker, status: {}", handshaker_name, status);
      return nullptr;
    }

    return std::make_unique<TsiHandshaker>(std::move(handshaker_ptr), dispatcher);
  };

  return std::make_unique<TsiSocketFactory>(factory, validator);
}

} // namespace

ProtobufTypes::MessagePtr S2ATransportSocketConfigFactory::createEmptyConfigProto() {
  return std::make_unique<envoy::extensions::transport_sockets::s2a::v3alpha::S2AConfiguration>();
}

Network::TransportSocketFactoryPtr
UpstreamS2ATransportSocketConfigFactory::createTransportSocketFactory(
    const Protobuf::Message& message,
    Server::Configuration::TransportSocketFactoryContext& factory_ctxt) {
  return createTransportSocketFactoryHelper(message, /* is_upstream */ true, factory_ctxt);
}

Network::TransportSocketFactoryPtr
DownstreamS2ATransportSocketConfigFactory::createTransportSocketFactory(
    const Protobuf::Message& message,
    Server::Configuration::TransportSocketFactoryContext& factory_ctxt,
    const std::vector<std::string>&) {
  return createTransportSocketFactoryHelper(message, /* is_upstream */ false, factory_ctxt);
}

REGISTER_FACTORY(UpstreamAltsTransportSocketConfigFactory,
                 Server::Configuration::UpstreamTransportSocketConfigFactory);

REGISTER_FACTORY(DownstreamAltsTransportSocketConfigFactory,
                 Server::Configuration::DownstreamTransportSocketConfigFactory);

} // namespace S2A
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy