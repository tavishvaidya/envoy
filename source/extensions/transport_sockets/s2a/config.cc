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

#include "src/core/tsi/s2a/grpc_s2a_credentials_options.h"

#include "absl/container/node_hash_set.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_join.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace S2A {

// smart pointer for grpc_s2a_credentials_options that will be automatically freed.
using GrpcS2ACredentialsOptionsPtr =
    CSmartPtr<grpc_s2a_credentials_options, grpc_s2a_credentials_options_destroy>;

namespace {

// Manage S2A singleton state via SingletonManager
class S2ASharedState : public Singleton::Instance {
public:
  S2ASharedState() { grpc_s2a_shared_resource_dedicated_init(); }

  ~S2ASharedState() override { grpc_s2a_shared_resource_dedicated_shutdown(); }

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

SINGLETON_MANAGER_REGISTRATION(s2a_shared_state);

Network::TransportSocketFactoryPtr createTransportSocketFactoryHelper(
    const Protobuf::Message& message, bool is_upstream,
    Server::Configuration::TransportSocketFactoryContext& factory_ctxt) {
  auto s2a_shared_state = factory_ctxt.singletonManager().getTyped<S2ASharedState>(
      SINGLETON_MANAGER_REGISTERED_NAME(s2a_shared_state),
      [] { return std::make_shared<S2ASharedState>(); });      
  auto config =
      MessageUtil::downcastAndValidate<const envoy::extensions::transport_sockets::s2a::v3alpha::S2AConfiguration&>(
          message, factory_ctxt.messageValidationVisitor());

  const std::string& handshaker_service = config.s2a_address();
  HandshakerFactory factory =
      [handshaker_service, is_upstream,
       s2a_shared_state](Event::Dispatcher& dispatcher,
                          const Network::Address::InstanceConstSharedPtr& local_address,
                          const Network::Address::InstanceConstSharedPtr&) -> TsiHandshakerPtr {
    ASSERT(local_address != nullptr);

    GrpcS2ACredentialsOptionsPtr options;
    options = GrpcS2ACredentialsOptionsPtr(grpc_s2a_credentials_options_create());
    const char* target_name = is_upstream ? "" : nullptr;
    tsi_handshaker* handshaker = nullptr;
    const std::string handshaker_name = is_upstream ? "client" : "server";
    // Specifying target name as empty since TSI won't take care of validating peer identity
    // in this use case. The validation will be performed by TsiSocket with the validator.
    // Set the max frame size to 16KB.
    S2ATsiHandshakerOptions s2a_options{nullptr, is_upstream, options.get(), handshaker_service};
    absl::StatusOr<tsi_handshaker*> s2a_handshaker = CreateS2ATsiHandshaker(s2a_options);
    
    CHandshakerPtr handshaker_ptr{handshaker};

    if (!s2a_handshaker.ok()) {
      const std::string handshaker_name = is_upstream ? "client" : "server";
      ENVOY_LOG_MISC(warn, "Cannot create S2A {} handshaker, status: {}", handshaker_name, s2a_handshaker.status());
      return nullptr;
    }

    return std::make_unique<TsiHandshaker>(std::move(*s2a_handshaker), dispatcher);
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

REGISTER_FACTORY(UpstreamS2ATransportSocketConfigFactory,
                 Server::Configuration::UpstreamTransportSocketConfigFactory);

REGISTER_FACTORY(DownstreamS2ATransportSocketConfigFactory,
                 Server::Configuration::DownstreamTransportSocketConfigFactory);

} // namespace S2A
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy