licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:public"])

load("//mediapipe/gpu:metal.bzl", "metal_library")

cc_library(
    name = "Mesh",
    srcs = ["Mesh.cc"],
    hdrs = ["Mesh.h","Primitive.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/gpu:gl_base",
        "//mediapipe/gpu:gl_simple_shaders",
        "//mediapipe/gpu:shader_util",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
    ],
)
cc_library(
    name = "Shader",
    srcs = ["Shader.cc"],
    hdrs = ["Shader.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/gpu:gl_base",
        "//mediapipe/gpu:gl_simple_shaders",
        "//mediapipe/gpu:shader_util",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
    ],
)
