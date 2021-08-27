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
#include "absl/status/statusor.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace S2A {
namespace {

// Manage S2A singleton state via SingletonManager
class S2ASharedState final : public Singleton::Instance {
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
  // A reference to this is held in the factory closure to keep the singleton
  // instance alive.
  auto s2a_shared_state = factory_ctxt.singletonManager().getTyped<S2ASharedState>(
      SINGLETON_MANAGER_REGISTERED_NAME(s2a_shared_state),
      [] { return std::make_shared<S2ASharedState>(); });
  auto config =
      MessageUtil::downcastAndValidate<const envoy::extensions::transport_sockets::s2a::v3alpha::S2AConfiguration&>(
          message, factory_ctxt.messageValidationVisitor());

  const std::string& s2a_address = config.s2a_address();
  HandshakerFactory factory =
      [s2a_address, is_upstream,
       s2a_shared_state](Event::Dispatcher& dispatcher,
                          const Network::Address::InstanceConstSharedPtr& local_address,
                          const Network::Address::InstanceConstSharedPtr&) -> TsiHandshakerPtr {
    // Merely using local_address here for proper Bazel build, variable needs to be used
    std::cout << "Entered lambda function." << std::endl;
    ASSERT(local_address != nullptr);
    std::cout << "Asserted local address." << std::endl;
    grpc_s2a_credentials_options* options = grpc_s2a_credentials_options_create();
    grpc_s2a_credentials_options_set_s2a_address(options, s2a_address.c_str());
    const char* target_name = is_upstream ? "" : nullptr; // TODO(djmoore): Populate target name
    s2a::tsi::S2ATsiHandshakerOptions s2a_options{/* interested_parties= */nullptr, is_upstream, options, target_name};
    absl::StatusOr<tsi_handshaker*> tsi_handshaker_ptr = CreateS2ATsiHandshaker(s2a_options);

    if (!tsi_handshaker_ptr.ok()) {
        // is_upstream is true if the stream of communication is with the client
      std::string handshaker_side = is_upstream ? "client" : "server";
      ENVOY_LOG_MISC(warn, "Cannot create S2A {} handshaker, status: {}", handshaker_side, tsi_handshaker_ptr.status());
      return nullptr;
    }

    CHandshakerPtr handshaker_ptr{*tsi_handshaker_ptr};

    return std::make_unique<TsiHandshaker>(std::move(*tsi_handshaker_ptr), dispatcher);
  };

  return std::make_unique<TsiSocketFactory>(factory, /*validator=*/ nullptr);
}

} // namespace

ProtobufTypes::MessagePtr S2ATransportSocketConfigFactory::createEmptyConfigProto() {
  return std::make_unique<envoy::extensions::transport_sockets::s2a::v3alpha::S2AConfiguration>();
}

Network::TransportSocketFactoryPtr
UpstreamS2ATransportSocketConfigFactory::createTransportSocketFactory(
    const Protobuf::Message& message,
    Server::Configuration::TransportSocketFactoryContext& factory_ctxt) {
  return createTransportSocketFactoryHelper(message, /*is_upstream=*/ true, factory_ctxt);
}

Network::TransportSocketFactoryPtr
DownstreamS2ATransportSocketConfigFactory::createTransportSocketFactory(
    const Protobuf::Message& message,
    Server::Configuration::TransportSocketFactoryContext& factory_ctxt,
    const std::vector<std::string>&) {
  return createTransportSocketFactoryHelper(message, /*is_upstream=*/ false, factory_ctxt);
}

REGISTER_FACTORY(UpstreamS2ATransportSocketConfigFactory,
                 Server::Configuration::UpstreamTransportSocketConfigFactory);

REGISTER_FACTORY(DownstreamS2ATransportSocketConfigFactory,
                 Server::Configuration::DownstreamTransportSocketConfigFactory);

} // namespace S2A
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy