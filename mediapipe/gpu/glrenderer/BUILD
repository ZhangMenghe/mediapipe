licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:public"])

load("//mediapipe/gpu:metal.bzl", "metal_library")

cc_library(
    name = "gl_point_renderer",
    srcs = ["gl_point_renderer.cc"],
    hdrs = ["gl_point_renderer.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/gpu:gl_base",
        "//mediapipe/gpu:gl_simple_shaders",
        "//mediapipe/gpu:shader_util",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
        "//mediapipe/gpu:scale_mode_cc_proto",
    ],
)