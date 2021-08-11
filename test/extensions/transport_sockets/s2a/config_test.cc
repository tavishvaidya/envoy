#include "source/common/protobuf/protobuf.h"
#include "source/common/singleton/manager_impl.h"
#include "source/extensions/transport_sockets/s2a/config.h"

#include "test/mocks/server/transport_socket_factory_context.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using Envoy::Server::Configuration::MockTransportSocketFactoryContext;
using testing::ReturnRef;

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace S2A {
namespace {

TEST(UpstreamS2AConfigTest, CreateSocketFactory) {
  NiceMock<MockTransportSocketFactoryContext> factory_context;
  Singleton::ManagerImpl singleton_manager{Thread::threadFactoryForTest()};
  EXPECT_CALL(factory_context, singletonManager()).WillRepeatedly(ReturnRef(singleton_manager));
  UpstreamS2ATransportSocketConfigFactory factory;

  ProtobufTypes::MessagePtr config = factory.createEmptyConfigProto();

  std::string yaml = R"EOF(
  s2a_address: 169.254.169.254:8080
  )EOF";
  TestUtility::loadFromYaml(yaml, *config);

  auto socket_factory = factory.createTransportSocketFactory(*config, factory_context);

  EXPECT_NE(nullptr, socket_factory);
  EXPECT_TRUE(socket_factory->implementsSecureTransport());
}

TEST(DownstreamS2AConfigTest, CreateSocketFactory) {
  NiceMock<MockTransportSocketFactoryContext> factory_context;
  Singleton::ManagerImpl singleton_manager{Thread::threadFactoryForTest()};
  EXPECT_CALL(factory_context, singletonManager()).WillRepeatedly(ReturnRef(singleton_manager));
  DownstreamS2ATransportSocketConfigFactory factory;

  ProtobufTypes::MessagePtr config = factory.createEmptyConfigProto();

  std::string yaml = R"EOF(
  s2a_address: 169.254.169.254:8080
  )EOF";
  TestUtility::loadFromYaml(yaml, *config);

  auto socket_factory = factory.createTransportSocketFactory(*config, factory_context, {});

  EXPECT_NE(nullptr, socket_factory);
  EXPECT_TRUE(socket_factory->implementsSecureTransport());
}

} // namespace
} // namespace S2A
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy
