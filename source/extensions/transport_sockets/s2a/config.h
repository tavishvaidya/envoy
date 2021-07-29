#pragma once

#include "envoy/server/transport_socket_config.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace S2A {

// S2A config registry
class S2ATransportSocketConfigFactory
    : public virtual Server::Configuration::TransportSocketConfigFactory {
public:
  ProtobufTypes::MessagePtr createEmptyConfigProto() override;
  std::string name() const override { return "envoy.transport_sockets.s2a"; }
};

class UpstreamS2ATransportSocketConfigFactory
    : public S2ATransportSocketConfigFactory,
      public Server::Configuration::UpstreamTransportSocketConfigFactory {
public:
  Network::TransportSocketFactoryPtr
  createTransportSocketFactory(const Protobuf::Message&,
                               Server::Configuration::TransportSocketFactoryContext&) override;
};

class DownstreamS2ATransportSocketConfigFactory
    : public S2ATransportSocketConfigFactory,
      public Server::Configuration::DownstreamTransportSocketConfigFactory {
public:
  Network::TransportSocketFactoryPtr
  createTransportSocketFactory(const Protobuf::Message&,
                               Server::Configuration::TransportSocketFactoryContext&,
                               const std::vector<std::string>&) override;
};

} // namespace S2A
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy