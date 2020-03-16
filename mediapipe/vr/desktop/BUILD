licenses(["notice"])  # Apache 2.0
package(default_visibility = ["//mediapipe/examples:__subpackages__"])
exports_files(["LICENSE"])

#vrNative
# cc_library(
#     name = "vrNative",
#     srcs = ["libvrNative.so"],
#     visibility = ["//visibility:public"],
# )

# Linux only.
# similar to demo_run_graph_main
# Must have a GPU with EGL support:
# ex: sudo aptitude install mesa-common-dev libegl1-mesa-dev libgles2-mesa-dev
# (or similar nvidia/amd equivalent)
cc_binary(
    name = "vr_dicom_gpu",
    srcs = ["vr_dicom.cc"],
    deps = [
        #usually for calculator
        "//mediapipe/calculators/image:luminance_calculator", 
        "//mediapipe/calculators/image:sobel_edges_calculator",

        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:commandlineflags",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:opencv_video",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/gpu:gl_calculator_helper",
        "//mediapipe/gpu:gpu_buffer",
        "//mediapipe/gpu:gpu_shared_data_internal",
    ],
)

cc_binary(
    name = "vr_dicom",
    srcs = ["vr_dicom.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/framework/port:logging",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
    ],
)
