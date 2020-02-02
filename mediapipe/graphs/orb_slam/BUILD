licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:public"])

load(
    "//mediapipe/framework/tool:mediapipe_graph.bzl",
    "mediapipe_binary_graph",
)

cc_library(
    name = "desktop_orbslam_calculators",
    deps = [
        "//mediapipe/calculators/slam:merge_slam_calculator",
        "//mediapipe/calculators/slam:orb_slam_calculator",
        "//mediapipe/calculators/image:image_transformation_calculator",
        "//mediapipe/calculators/image:luminance_calculator",
        "//mediapipe/gpu:gpu_buffer_to_image_frame_calculator",
    ],
)