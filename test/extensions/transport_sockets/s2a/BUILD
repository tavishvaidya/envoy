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
    name = "tsi_frame_protector_test",
    srcs = ["tsi_frame_protector_test.cc"],
    extension_names = ["envoy.transport_sockets.s2a"],
    deps = [
        "//source/extensions/transport_sockets/s2a:tsi_frame_protector",
        "//test/mocks/buffer:buffer_mocks",
    ],
)
