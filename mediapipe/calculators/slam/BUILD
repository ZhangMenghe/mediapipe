licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

exports_files(["LICENSE"])
load("//mediapipe/framework/port:build_config.bzl", "mediapipe_cc_proto_library")

cc_library(
    name = "helmsley_vr",
    srcs = ["helmsley_vr.cc"],
    hdrs = ["helmsley_vr.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/util:resource_util",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
    ]+ select({
        "//mediapipe/gpu:disable_gpu": [],
        "//conditions:default": [
            "@helmsley_vr//:vrNative",

            "@linux_usr//:glm",
            
        "//mediapipe/gpu/glPipeline:Shader",
        "//mediapipe/gpu/glPipeline:Mesh",
            "//mediapipe/gpu:gl_simple_shaders",


            "//mediapipe/gpu:shader_util",    


                    "//mediapipe/gpu:gl_base",

        ],
    }),
    alwayslink = 1,
)

cc_library(
    name = "merge_slam_calculator",
    srcs = ["merge_slam_calculator.cc"],
    hdrs = ["slam_calculators.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:opencv_core",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_video",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
    ]+ select({
        "//mediapipe/gpu:disable_gpu": [],
        "//conditions:default": [
            ":helmsley_vr",
            # "@helmsley_vr//:HELMSLEY_VR",

            "@linux_usr//:glm",
            
            "//mediapipe/gpu:gl_shader_helper_cc_proto",
            "//mediapipe/gpu:gl_calculator_helper",
            "//mediapipe/gpu:gl_simple_shaders",
            "//mediapipe/gpu/glrenderer:gl_point_renderer",
            "//mediapipe/gpu/glrenderer:gl_cube_renderer",
           "//mediapipe/gpu/glrenderer:gl_arbg_renderer",

            "//mediapipe/gpu:shader_util",
            "//mediapipe/util:resource_util",
        
        ],
    }),
    alwayslink = 1,
)
cc_library(
    name = "orb_slam_calculator",
    srcs = ["orb_slam_calculator.cc"],
    hdrs = ["slam_calculators.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/util:resource_util",
        ":orb_slam_calculator_cc_proto",
        "@orb_slam2//:ORB_SLAM2",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:opencv_core",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
    ]+ select({
        "//mediapipe/gpu:disable_gpu": [],
        "//conditions:default": [
            "@linux_usr//:glm",
            "//mediapipe/gpu:gl_calculator_helper",
            "//mediapipe/gpu:gl_simple_shaders",
            # "//mediapipe/gpu:gl_quad_renderer",
            "//mediapipe/gpu:shader_util",
        ],
    }),
    alwayslink = 1,
)
mediapipe_cc_proto_library(
    name = "orb_slam_calculator_cc_proto",
    srcs = ["orb_slam_calculator.proto"],
    cc_deps = ["//mediapipe/framework:calculator_cc_proto"],
    visibility = ["//visibility:public"],
    deps = [":orb_slam_calculator_proto"],
)
proto_library(
    name = "orb_slam_calculator_proto",
    srcs = ["orb_slam_calculator.proto"],
    visibility = ["//visibility:public"],
    deps = ["//mediapipe/framework:calculator_proto"],
)
