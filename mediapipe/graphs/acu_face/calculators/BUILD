#Borrowed from hand_tracking calculators

licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "ear_rects_from_face_landmarks_calculator",
    srcs = ["ear_rects_from_face_landmarks_calculator.cc"],
    visibility = ["//visibility:public"],
    deps = [
        # "//mediapipe/calculators/util:detections_to_rects_calculator",
        # "//mediapipe/calculators/util:detections_to_rects_calculator_cc_proto",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework:calculator_options_cc_proto",
        "//mediapipe/framework/formats:detection_cc_proto",
        "//mediapipe/framework/formats:location_data_cc_proto",
        "//mediapipe/framework/formats:rect_cc_proto",
        "//mediapipe/framework/formats:landmark_cc_proto",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
            "//mediapipe/gpu:gl_calculator_helper",

    ],
    alwayslink = 1,
)

cc_library(
    name = "ear_landmarks_to_rects_calculator",
    srcs = ["ear_landmarks_to_rects_calculator.cc"],
    visibility = ["//visibility:public"],
    deps = [
        # ":landmarks_to_floats_calculator_cc_proto",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:landmark_cc_proto",
        "//mediapipe/framework/formats:rect_cc_proto",
        "//mediapipe/framework/formats:matrix",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
        "@eigen_archive//:eigen",
    ],
    alwayslink = 1,
)
cc_library(
    name = "save_cpu_image_calculator",
    srcs = ["save_cpu_image_calculator.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework/port:opencv_core",
        "//mediapipe/framework/port:opencv_highgui",

        "//mediapipe/gpu:gl_calculator_helper",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework:timestamp",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",

        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
    ],
    alwayslink = 1,
)
cc_library(
    name = "fine_crop_ear_cpu_calculator",
    srcs = ["fine_crop_ear_cpu_calculator.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework/formats:rect_cc_proto",

        "//mediapipe/framework/port:opencv_core",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",

        "//mediapipe/gpu:gl_calculator_helper",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework:timestamp",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",

        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
    ],
    alwayslink = 1,
)
cc_library(
    name = "ear_normalize_calculator",
    srcs = ["ear_normalize_calculator.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework/port:opencv_core",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",

        "//mediapipe/gpu:gl_calculator_helper",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework:timestamp",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",

        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:status",
    ],
    alwayslink = 1,
)
cc_library(
    name = "tflite_tensors_to_flipped_ear",
    srcs = ["tflite_tensors_to_flipped_ear.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework/port:opencv_core",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",

        "//mediapipe/framework:timestamp",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:ret_check",
        "@org_tensorflow//tensorflow/lite:framework",
    ],
    alwayslink = 1,
)
cc_library(
    name = "tflite_tensors_to_adjust_ear_observer",
    srcs = ["tflite_tensors_to_adjust_ear_observer.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework/port:opencv_core",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",

        "//mediapipe/framework:timestamp",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:ret_check",
        "@org_tensorflow//tensorflow/lite:framework",
    ],
    alwayslink = 1,
)