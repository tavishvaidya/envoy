#include <chrono>

#include "source/common/protobuf/protobuf.h"
#include "source/common/singleton/manager_impl.h"
#include "source/extensions/transport_sockets/s2a/config.h"
#include "source/extensions/transport_sockets/s2a/tsi_socket.h"


#include "test/mocks/server/transport_socket_factory_context.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace TransportSockets {
namespace S2A {
namespace {

using Envoy::Server::Configuration::MockTransportSocketFactoryContext;
using testing::ReturnRef;

class TestTimeSource : public TimeSource {
public:
    TestTimeSource() {}
   ~TestTimeSource() override {}

  /**
   * @return the current system time; not guaranteed to be monotonically increasing.
   */
   SystemTime systemTime() override {
       return std::chrono::system_clock::now();
   };
  /**
   * @return the current monotonic time.
   */
   MonotonicTime monotonicTime() override {
       return std::chrono::steady_clock::now();
   };
};

class TestDispatcher : public Event::Dispatcher {
public:
    TestDispatcher() {}
    ~TestDispatcher() override {}
  /**
   * Returns the name that identifies this dispatcher, such as "worker_2" or "main_thread".
   * @return const std::string& the name that identifies this dispatcher.
   */
   const std::string& name() override {
      return "";
  };

  /**
   * Creates a file event that will signal when a file is readable or writable. On UNIX systems this
   * can be used for any file like interface (files, sockets, etc.).
   * @param fd supplies the fd to watch.
   * @param cb supplies the callback to fire when the file is ready.
   * @param trigger specifies whether to edge or level trigger.
   * @param events supplies a logical OR of FileReadyType events that the file event should
   *               initially listen on.
   */
   Event::FileEventPtr createFileEvent(os_fd_t, Event::FileReadyCb, Event::FileTriggerType,
                                       uint32_t) override{
                                           return nullptr;
                                       };

  /**
   * Allocates a timer. @see Timer for docs on how to use the timer.
   * @param cb supplies the callback to invoke when the timer fires.
   */
   Event::TimerPtr createTimer(Event::TimerCb) override {
      return nullptr;
  };

  /**
   * Allocates a scaled timer. @see Timer for docs on how to use the timer.
   * @param timer_type the type of timer to create.
   * @param cb supplies the callback to invoke when the timer fires.
   */
   Event::TimerPtr createScaledTimer(Event::ScaledTimerType, Event::TimerCb) override {
      return nullptr;
  };

  /**
   * Allocates a scaled timer. @see Timer for docs on how to use the timer.
   * @param minimum the rule for computing the minimum value of the timer.
   * @param cb supplies the callback to invoke when the timer fires.
   */
   Event::TimerPtr createScaledTimer(Event::ScaledTimerMinimum, Event::TimerCb) override{
      return nullptr;
  };

  /**
   * Allocates a schedulable callback. @see SchedulableCallback for docs on how to use the wrapped
   * callback.
   * @param cb supplies the callback to invoke when the SchedulableCallback is triggered on the
   * event loop.
   */
   Event::SchedulableCallbackPtr createSchedulableCallback(std::function<void()>) override {
      return nullptr;
  };

  /**
   * Register a watchdog for this dispatcher. The dispatcher is responsible for touching the
   * watchdog at least once per touch interval. Dispatcher implementations may choose to touch more
   * often to avoid spurious miss events when processing long callback queues.
   * @param min_touch_interval Touch interval for the watchdog.
   */
   void registerWatchdog(const Server::WatchDogSharedPtr&,
                                std::chrono::milliseconds) override {
                                    return;
                                };

  /**
   * Returns a time-source to use with this dispatcher.
   */
   TimeSource& timeSource() override {return *TestTimeSource();};

  /**
   * Returns a recently cached MonotonicTime value.
   */
   MonotonicTime approximateMonotonicTime() const override {
       return std::chrono::steady_clock::now();
   };

  /**
   * Initializes stats for this dispatcher. Note that this can't generally be done at construction
   * time, since the main and worker thread dispatchers are constructed before
   * ThreadLocalStoreImpl::initializeThreading.
   * @param scope the scope to contain the new per-dispatcher stats created here.
   * @param prefix the stats prefix to identify this dispatcher. If empty, the dispatcher will be
   *               identified by its name.
   */
   void initializeStats(Stats::Scope&,
                               const absl::optional<std::string>&) override {
                                   return;
                               };

  /**
   * Clears any items in the deferred deletion queue.
   */
   void clearDeferredDeleteList() override {
       return;
   };

  /**
   * Wraps an already-accepted socket in an instance of Envoy's server Network::Connection.
   * @param socket supplies an open file descriptor and connection metadata to use for the
   *        connection. Takes ownership of the socket.
   * @param transport_socket supplies a transport socket to be used by the connection.
   * @param stream_info info object for the server connection
   * @return Network::ConnectionPtr a server connection that is owned by the caller.
   */
   Network::ServerConnectionPtr
  createServerConnection(Network::ConnectionSocketPtr&&,
                         Network::TransportSocketPtr&&,
                         StreamInfo::StreamInfo&) override {
                            return nullptr;
                         };

  /**
   * Creates an instance of Envoy's Network::ClientConnection. Does NOT initiate the connection;
   * the caller must then call connect() on the returned Network::ClientConnection.
   * @param address supplies the address to connect to.
   * @param source_address supplies an address to bind to or nullptr if no bind is necessary.
   * @param transport_socket supplies a transport socket to be used by the connection.
   * @param options the socket options to be set on the underlying socket before anything is sent
   *        on the socket.
   * @return Network::ClientConnectionPtr a client connection that is owned by the caller.
   */
   Network::ClientConnectionPtr
  createClientConnection(Network::Address::InstanceConstSharedPtr,
                         Network::Address::InstanceConstSharedPtr,
                         Network::TransportSocketPtr&&,
                         const Network::ConnectionSocket::OptionsSharedPtr&) override {
                             return nullptr;
                         };

  /**
   * Creates an async DNS resolver. The resolver should only be used on the thread that runs this
   * dispatcher.
   * @param resolvers supplies the addresses of DNS resolvers that this resolver should use. If left
   * empty, it will not use any specific resolvers, but use defaults (/etc/resolv.conf)
   * @param dns_resolver_options supplies the aggregated area options flags needed for dns resolver
   * init.
   * @return Network::DnsResolverSharedPtr that is owned by the caller.
   */
   Network::DnsResolverSharedPtr
  createDnsResolver(const std::vector<Network::Address::InstanceConstSharedPtr>&,
                    const envoy::config::core::v3::DnsResolverOptions&) override {
                        return nullptr;
                    };

  /**
   * @return Filesystem::WatcherPtr a filesystem watcher owned by the caller.
   */
   Filesystem::WatcherPtr createFilesystemWatcher() override {
       return nullptr;
   };

  /**
   * Creates a listener on a specific port.
   * @param socket supplies the socket to listen on.
   * @param cb supplies the callbacks to invoke for listener events.
   * @param bind_to_port controls whether the listener binds to a transport port or not.
   * @return Network::ListenerPtr a new listener that is owned by the caller.
   */
   Network::ListenerPtr createListener(Network::SocketSharedPtr&&,
                                              Network::TcpListenerCallbacks&,
                                              bool) override {
                                                  return nullptr;
                                              };

  /**
   * Creates a logical udp listener on a specific port.
   * @param socket supplies the socket to listen on.
   * @param cb supplies the udp listener callbacks to invoke for listener events.
   * @param config provides the UDP socket configuration.
   * @return Network::ListenerPtr a new listener that is owned by the caller.
   */
   Network::UdpListenerPtr
  createUdpListener(Network::SocketSharedPtr, Network::UdpListenerCallbacks&,
                    const envoy::config::core::v3::UdpSocketConfig&) override {
                        return nullptr;
                    };
  /**
   * Submits an item for deferred delete. @see DeferredDeletable.
   */
   void deferredDelete(Event::DeferredDeletablePtr&&) override {
       return;
   };

  /**
   * Exits the event loop.
   */
   void exit() override {
       return;
   };

  /**
   * Listens for a signal event. Only a single dispatcher in the process can listen for signals.
   * If more than one dispatcher calls this routine in the process the behavior is undefined.
   *
   * @param signal_num supplies the signal to listen on.
   * @param cb supplies the callback to invoke when the signal fires.
   * @return SignalEventPtr a signal event that is owned by the caller.
   */
   Event::SignalEventPtr listenForSignal(signal_t, Event::SignalCb) override {
       return nullptr;
   };

  /**
   * Post the deletable to this dispatcher. The deletable objects are guaranteed to be destroyed on
   * the dispatcher's thread before dispatcher destroy. This is safe cross thread.
   */
   void deleteInDispatcherThread(Event::DispatcherThreadDeletableConstPtr) override {
       return;
   };

  /**
   * Runs the event loop. This will not return until exit() is called either from within a callback
   * or from a different thread.
   * @param type specifies whether to run in blocking mode (run() will not return until exit() is
   *              called) or non-blocking mode where only active events will be executed and then
   *              run() will return.
   */
  enum class RunType {
    Block,       // Runs the event-loop until there are no pending events.
    NonBlock,    // Checks for any pending events to activate, executes them,
                 // then exits. Exits immediately if there are no pending or
                 // active events.
    RunUntilExit // Runs the event-loop until loopExit() is called, blocking
                 // until there are pending or active events.
  };
   void run(RunType type) override {
       return;
   };

  /**
   * Updates approximate monotonic time to current value.
   */
   void updateApproximateMonotonicTime() override {
       return;
   };

  /**
   * Shutdown the dispatcher by clear dispatcher thread deletable.
   */
   void shutdown() override {
       return;
   };
};

class TestSocketAddressProvider: public Network::SocketAddressProvider {
public:
    TestSocketAddressProvider()  {}
    ~TestSocketAddressProvider() override {}

  /**
   * @return the local address of the socket.
   */
   const Network::Address::InstanceConstSharedPtr& localAddress() const override {
       return nullptr;
   };

  /**
   * @return true if the local address has been restored to a value that is different from the
   *         address the socket was initially accepted at.
   */
   bool localAddressRestored() const override {
       return true;
   };

  /**
   * @return the remote address of the socket.
   */
   const Network::Address::InstanceConstSharedPtr& remoteAddress() const override {
       return nullptr;
   };

  /**
   * @return the direct remote address of the socket. This is the address of the directly
   *         connected peer, and cannot be modified by listener filters.
   */
   const Network::Address::InstanceConstSharedPtr& directRemoteAddress() const override {
       return nullptr;
   };

  /**
   * @return SNI value for downstream host.
   */
   absl::string_view requestedServerName() const override {
       return "";
   };

  /**
   * @return Connection ID of the downstream connection, or unset if not available.
   **/
   absl::optional<uint64_t> connectionID() const override {
       return absl::nullopt;
   };

  /**
   * Dumps the state of the SocketAddressProvider to the given ostream.
   *
   * @param os the std::ostream to dump to.
   * @param indent_level the level of indentation.
   */
   void dumpState(std::ostream&, int) const override {
       return;
   };
  };

class TestConnection: public Envoy::Network::Connection {
public:
    // constructor; actually functional
    // need private member variable of of the TestAddressProvider
    TestConnection() {}
    ~TestConnection() override {}
  /**
   * Register callbacks that fire when connection events occur.
   */
   void addConnectionCallbacks(Network::ConnectionCallbacks& cb) override {
       return;
   };

  /**
   * Unregister callbacks which previously fired when connection events occur.
   */
   void removeConnectionCallbacks(Network::ConnectionCallbacks& cb) override {
       return;
   };

  /**
   * Register for callback every time bytes are written to the underlying TransportSocket.
   */
   void addBytesSentCallback(BytesSentCb cb) override {
       return;
   };

  /**
   * Enable half-close semantics on this connection. Reading a remote half-close
   * will not fully close the connection. This is off by default.
   * @param enabled Whether to set half-close semantics as enabled or disabled.
   */
   void enableHalfClose(bool enabled) override {
       return;
   };

  /**
   * @return true if half-close semantics are enabled, false otherwise.
   */
   bool isHalfCloseEnabled() override {
       return true;
   };

  /**
   * Close the connection.
   */
   void close(Network::ConnectionCloseType type) override {
       return;
   };

  /**
   * @return Event::Dispatcher& the dispatcher backing this connection.
   */
   Event::Dispatcher& dispatcher() override {
       return;
   };

  /**
   * @return uint64_t the unique local ID of this connection.
   */
   uint64_t id() const override {
       return 0;
   };

  /**
   * @param vector of bytes to which the connection should append hash key data. Any data already in
   * the key vector must not be modified.
   */
   void hashKey(std::vector<uint8_t>& hash) const override {
       return;
   };

  /**
   * @return std::string the next protocol to use as selected by network level negotiation. (E.g.,
   *         ALPN). If network level negotiation is not supported by the connection or no protocol
   *         has been negotiated the empty string is returned.
   */
   std::string nextProtocol() const override {
       return "";
   };

  /**
   * Enable/Disable TCP NO_DELAY on the connection.
   */
   void noDelay(bool enable) override {
       return;
   };

  /**
   * Disable socket reads on the connection, applying external back pressure. When reads are
   * enabled again if there is data still in the input buffer it will be re-dispatched through
   * the filter chain.
   * @param disable supplies TRUE is reads should be disabled, FALSE if they should be enabled.
   *
   * Note that this function reference counts calls. For example
   * readDisable(true);  // Disables data
   * readDisable(true);  // Notes the connection is blocked by two sources
   * readDisable(false);  // Notes the connection is blocked by one source
   * readDisable(false);  // Marks the connection as unblocked, so resumes reading.
   */
   void readDisable(bool disable) override {
       return;
   };

  /**
   * Set if Envoy should detect TCP connection close when readDisable(true) is called.
   * By default, this is true on newly created connections.
   *
   * @param should_detect supplies if disconnects should be detected when the connection has been
   * read disabled
   */
   void detectEarlyCloseWhenReadDisabled(bool should_detect) override {
       return;
   };

  /**
   * @return bool whether reading is enabled on the connection.
   */
   bool readEnabled() const override {
       return true;
   };

  /**
   * @return the address provider backing this connection.
   */
   const SocketAddressProvider& addressProvider() const override {
       return testAddress;
   };;
   Network::SocketAddressProviderSharedPtr addressProviderSharedPtr() const override {
       return testAddress;
   };;

  /**
   * Credentials of the peer of a socket as decided by SO_PEERCRED.
   */
  struct UnixDomainSocketPeerCredentials {
    /**
     * The process id of the peer.
     */
    int32_t pid;
    /**
     * The user id of the peer.
     */
    uint32_t uid;
    /**
     * The group id of the peer.
     */
    uint32_t gid;
  };

  /**
   * @return The unix socket peer credentials of the remote client. Note that this is only
   * supported for unix socket connections.
   */
   absl::optional<Network::Connection::UnixDomainSocketPeerCredentials> unixSocketPeerCredentials() const override {
       return absl::nullopt;
   };

  /**
   * Set the stats to update for various connection state changes. Note that for performance reasons
   * these stats are eventually consistent and may not always accurately represent the connection
   * state at any given point in time.
   */
   void setConnectionStats(const ConnectionStats& stats) override {
       return;
   };

  /**
   * @return the const SSL connection data if this is an SSL connection, or nullptr if it is not.
   */
  // TODO(snowp): Remove this in favor of StreamInfo::downstreamSslConnection.
   Ssl::ConnectionInfoConstSharedPtr ssl() const override {
       return nullptr;
   };

  /**
   * @return requested server name (e.g. SNI in TLS), if any.
   */
   absl::string_view requestedServerName() const override {
       return "";
   };

  /**
   * @return State the current state of the connection.
   */
   State state() const override {
       return nullptr;
   };

  /**
   * @return true if the connection has not completed connecting, false if the connection is
   * established.
   */
   bool connecting() const override {
       return true;
   };

  /**
   * Write data to the connection. Will iterate through downstream filters with the buffer if any
   * are installed.
   * @param data Supplies the data to write to the connection.
   * @param end_stream If true, this indicates that this is the last write to the connection. If
   *        end_stream is true, the connection is half-closed. This may only be set to true if
   *        enableHalfClose(true) has been set on this connection.
   */
   void write(Buffer::Instance& data, bool end_stream) override {
       return;
   };

  /**
   * Set a soft limit on the size of buffers for the connection.
   * For the read buffer, this limits the bytes read prior to flushing to further stages in the
   * processing pipeline.
   * For the write buffer, it sets watermarks. When enough data is buffered it triggers a call to
   * onAboveWriteBufferHighWatermark, which allows subscribers to enforce flow control by disabling
   * reads on the socket funneling data to the write buffer. When enough data is drained from the
   * write buffer, onBelowWriteBufferHighWatermark is called which similarly allows subscribers
   * resuming reading.
   */
   void setBufferLimits(uint32_t limit) override {
       return;
   };

  /**
   * Get the value set with setBufferLimits.
   */
   uint32_t bufferLimit() const override {
       return 0;
   };

  /**
   * @return boolean telling if the connection is currently above the high watermark.
   */
   bool aboveHighWatermark() const override {
       return true;
   };

  /**
   * Get the socket options set on this connection.
   */
   const Network::ConnectionSocket::OptionsSharedPtr& socketOptions() const override {
       return nullptr;
   };

  /**
   * The StreamInfo object associated with this connection. This is typically
   * used for logging purposes. Individual filters may add specific information
   * via the FilterState object within the StreamInfo object. The StreamInfo
   * object in this context is one per connection i.e. different than the one in
   * the http ConnectionManager implementation which is one per request.
   *
   * @return StreamInfo object associated with this connection.
   */
   virtual StreamInfo::StreamInfo& streamInfo() PURE;
   virtual const StreamInfo::StreamInfo& streamInfo() const PURE;

  /**
   * Set the timeout for delayed connection close()s.
   * This can only be called prior to issuing a close() on the connection.
   * @param timeout The timeout value in milliseconds
   */
   void setDelayedCloseTimeout(std::chrono::milliseconds timeout) override {
       return;
   };

  /**
   * @return std::string the failure reason of the underlying transport socket, if no failure
   *         occurred an empty string is returned.
   */
   absl::string_view transportFailureReason() const override {
       return true;
   };

  /**
   * Instructs the connection to start using secure transport.
   * Note: Not all underlying transport sockets support such operation.
   * @return boolean telling if underlying transport socket was able to
             start secure transport.
   */
   bool startSecureTransport() override {
       return true;
   };

  /**
   *  @return absl::optional<std::chrono::milliseconds> An optional of the most recent round-trip
   *  time of the connection. If the platform does not support this, then an empty optional is
   *  returned.
   */
   absl::optional<std::chrono::milliseconds> lastRoundTripTime() const override {
       return absl::nullopt;
   };
private:
    TestSocketAddressProvider testAddress;
};

class TestTransportSocketCallbacks: public Envoy::Network::TransportSocketCallbacks {
public:
  /**
   * @return Network::Connection& the connection interface.
   */
   Network::Connection& connection() override {
       return *testConn;
   };

private:
    TestConnection testConn;
};

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

  Network::TransportSocketFactoryPtr socket_factory = factory.createTransportSocketFactory(*config, factory_context);

  EXPECT_NE(nullptr, socket_factory);
  EXPECT_TRUE(socket_factory->implementsSecureTransport());

  auto tsi_socket = socket_factory->createTransportSocket(nullptr);
  tsi_socket->onConnected();
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

  Network::TransportSocketFactoryPtr socket_factory = factory.createTransportSocketFactory(*config, factory_context, {});

  EXPECT_NE(nullptr, socket_factory);
  EXPECT_TRUE(socket_factory->implementsSecureTransport());
}

} // namespace
} // namespace S2A
} // namespace TransportSockets
} // namespace Extensions
} // namespace Envoy
