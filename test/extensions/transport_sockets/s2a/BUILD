load(
    "//bazel:envoy_build_system.bzl",
    "envoy_package",
    "envoy_select_google_grpc",
)
load(
    "//test/extensions:extensions_build_system.bzl",
    "envoy_extension_cc_test",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_extension_cc_test(
    name = "config_test",
    srcs = ["config_test.cc"],
    extension_names = ["envoy.transport_sockets.s2a"],
    deps = [
        "//source/common/singleton:manager_impl_lib",
        "//source/extensions/transport_sockets/s2a:config",
        "//test/mocks/event:event_mocks",
        "//test/mocks/network:network_mocks",
        "//test/mocks/server:transport_socket_factory_context_mocks",
    ],
)

envoy_extension_cc_test(
    name = "tsi_frame_protector_test",
    srcs = ["tsi_frame_protector_test.cc"],
    extension_names = ["envoy.transport_sockets.s2a"],
    deps = [
        "//source/extensions/transport_sockets/s2a:tsi_frame_protector",
        "//test/mocks/buffer:buffer_mocks",
    ],
)

envoy_extension_cc_test(
    name = "tsi_handshaker_test",
    srcs = ["tsi_handshaker_test.cc"],
    extension_names = ["envoy.transport_sockets.s2a"],
    deps = [
        "//envoy/event:dispatcher_interface",
        "//source/extensions/transport_sockets/s2a:tsi_handshaker",
        "//test/mocks/buffer:buffer_mocks",
        "//test/mocks/event:event_mocks",
    ],
)

envoy_extension_cc_test(
    name = "tsi_socket_test",
    srcs = ["tsi_socket_test.cc"],
    extension_names = ["envoy.transport_sockets.s2a"],
    deps = [
        "//envoy/event:dispatcher_interface",
        "//source/extensions/transport_sockets/s2a:tsi_socket",
        "//test/mocks/buffer:buffer_mocks",
        "//test/mocks/event:event_mocks",
        "//test/mocks/network:network_mocks",
    ],
)

envoy_extension_cc_test(
    name = "noop_transport_socket_callbacks_test",
    srcs = ["noop_transport_socket_callbacks_test.cc"],
    extension_names = ["envoy.transport_sockets.s2a"],
    deps = [
        "//source/extensions/transport_sockets/s2a:noop_transport_socket_callbacks_lib",
        "//test/mocks/network:network_mocks",
    ],
)
