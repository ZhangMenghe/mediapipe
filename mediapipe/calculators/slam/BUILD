licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

exports_files(["LICENSE"])

cc_library(
    name = "orb_calculator",
    srcs = ["orb_calculator.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "@orb_slam2//:ORB_SLAM2",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
    ],
    alwayslink = 1,
)
