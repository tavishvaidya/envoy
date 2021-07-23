load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_extension",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "grpc_tsi_wrapper",
    hdrs = [
        "grpc_tsi.h",
    ],
    external_deps = [
        "grpc",
    ],
    visibility = ["//visibility:private"],
    deps = [
        "//source/common/common:c_smart_ptr_lib",
    ],
)

envoy_cc_library(
    name = "tsi_frame_protector",
    srcs = [
        "tsi_frame_protector.cc",
    ],
    hdrs = [
        "tsi_frame_protector.h",
    ],
    deps = [
        ":grpc_tsi_wrapper",
        "//source/common/buffer:buffer_lib",
    ],
)