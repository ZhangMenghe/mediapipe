licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

exports_files(["LICENSE"])

cc_library(
    name = "orb_slam_calculator",
    srcs = [
        "merge_slam_calculator.cc",
        "orb_slam_calculator.cc"
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@orb_slam2//:ORB_SLAM2",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
    ]+ select({
        "//mediapipe/gpu:disable_gpu": [],
        "//conditions:default": [
            "//mediapipe/gpu:gl_calculator_helper",
            "//mediapipe/gpu:gl_simple_shaders",
            "//mediapipe/gpu:gl_quad_renderer",
            "//mediapipe/gpu:shader_util",
        ],
    }),
    alwayslink = 1,
)
