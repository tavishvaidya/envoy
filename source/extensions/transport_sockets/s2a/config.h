#pragma once

#include "envoy/server/transport_socket_config.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace S2A {

// S2ATransportSocketConfigFactory provides functions that contribute to the testing of
// Upstream and Downstream S2A Transport Socket Config Factories
class S2ATransportSocketConfigFactory
    : public virtual Server::Configuration::TransportSocketConfigFactory {
public:
  // For testing purposes
  ProtobufTypes::MessagePtr createEmptyConfigProto() override;
  std::string name() const override { return "envoy.transport_sockets.s2a"; }
};

// Registers a Transport Socket Factory for upstream S2A communication
class UpstreamS2ATransportSocketConfigFactory
    : public S2ATransportSocketConfigFactory,
      public Server::Configuration::UpstreamTransportSocketConfigFactory {
public:
  Network::TransportSocketFactoryPtr
  createTransportSocketFactory(const Protobuf::Message&,
                               Server::Configuration::TransportSocketFactoryContext&) override;
};

// Registers a Transport Socket Factory for downstream S2A communication
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