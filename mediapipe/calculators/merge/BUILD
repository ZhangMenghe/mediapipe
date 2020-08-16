licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

exports_files(["LICENSE"])
load("//mediapipe/framework/port:build_config.bzl", "mediapipe_cc_proto_library")
cc_library(
    name = "face_acu_generator",
    srcs = ["acu_generator.cc"],
    hdrs = ["acu_generator.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework/port:opencv_core",
        "//mediapipe/util:resource_util",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
    ]+ select({
        "//mediapipe/gpu:disable_gpu": [],
        "//conditions:default": [
            "@linux_usr//:glm",
            "@acu_face_render//:acuRenderer",
            "//mediapipe/gpu:shader_util",    
            "//mediapipe/gpu:gl_base",
        ],
    }),
    alwayslink = 1,
)
cc_library(
    name = "merge_face_calculator",
    srcs = ["merge_face_calculator.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/calculators/core:constant_side_packet_calculator",
        
        "//mediapipe/calculators/image:recolor_calculator_cc_proto",
        "//mediapipe/calculators/util:landmarks_to_render_data_calculator",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/formats:rect_cc_proto",
        "//mediapipe/framework/formats:detection_cc_proto",
        "//mediapipe/util:render_data_cc_proto",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/util:color_cc_proto",
        "//mediapipe/framework/port:opencv_core",
        "//mediapipe/framework/port:opencv_imgproc",
    ] + select({
        "//mediapipe/gpu:disable_gpu": [],
        "//conditions:default": [
            ":face_acu_generator",
            "//mediapipe/gpu/glrenderer:gl_arbg_renderer",
            "//mediapipe/gpu/glrenderer:gl_point_renderer",
            "//mediapipe/gpu:gl_calculator_helper",
            "//mediapipe/gpu:gl_shader_helper_cc_proto",
            "//mediapipe/gpu:gl_simple_shaders",
            "//mediapipe/gpu:gl_quad_renderer",
            "//mediapipe/gpu:shader_util",
        ],
    }),
    alwayslink = 1,
)
