load(
    "//mediapipe/framework/tool:mediapipe_graph.bzl",
    "mediapipe_simple_subgraph",
)

licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:public"])

mediapipe_simple_subgraph(
    name = "hair_segmentation_gpu",
    graph = "hair_segmentation_gpu.pbtxt",
    register_as = "HairSegmentationGpuSubgraph",
    deps = [
        "//mediapipe/calculators/core:flow_limiter_calculator",
        "//mediapipe/calculators/core:previous_loopback_calculator",
        "//mediapipe/calculators/image:image_transformation_calculator",
        "//mediapipe/calculators/image:recolor_calculator",
        "//mediapipe/calculators/image:set_alpha_calculator",
        "//mediapipe/calculators/tflite:tflite_converter_calculator",
        "//mediapipe/calculators/tflite:tflite_custom_op_resolver_calculator",
        "//mediapipe/calculators/tflite:tflite_inference_calculator",
        "//mediapipe/calculators/tflite:tflite_tensors_to_segmentation_calculator",
        "//mediapipe/gpu:gpu_buffer_to_image_frame_calculator",
        "//mediapipe/gpu:image_frame_to_gpu_buffer_calculator",
    ],
)

mediapipe_simple_subgraph(
    name = "make_sure_ear_left",
    graph = "make_sure_ear_left.pbtxt",
    register_as = "MakeSureEarLeftSubgraph",
    deps = [
        "//mediapipe/calculators/image:image_cropping_calculator",
        "//mediapipe/calculators/image:image_transformation_calculator",
        "//mediapipe/calculators/tflite:tflite_converter_calculator",
        "//mediapipe/calculators/tflite:tflite_custom_op_resolver_calculator",
        "//mediapipe/calculators/tflite:tflite_inference_calculator",
        "//mediapipe/calculators/tflite:tflite_tensors_to_landmarks_calculator",
        "//mediapipe/calculators/util:landmark_projection_calculator",
        "//mediapipe/graphs/acu_face/calculators:ear_landmarks_to_rects_calculator",
        "//mediapipe/calculators/util:rect_transformation_calculator",
        "//mediapipe/gpu:gpu_buffer_to_image_frame_calculator",
        "//mediapipe/gpu:image_frame_to_gpu_buffer_calculator",
        "//mediapipe/graphs/acu_face/calculators:tflite_tensors_to_flipped_ear",
    ],
)
mediapipe_simple_subgraph(
    name = "stage_one_ear_landmarkd_detection",
    graph = "stage_one_ear_landmarkd_detection.pbtxt",
    register_as = "StageOneEarLandmarkDetectionSubgraph",
    deps = [
        "//mediapipe/calculators/tflite:tflite_converter_calculator",
        "//mediapipe/calculators/tflite:tflite_custom_op_resolver_calculator",
        "//mediapipe/calculators/tflite:tflite_inference_calculator",
        # "//mediapipe/calculators/tflite:tflite_tensors_to_landmarks_calculator",
        "//mediapipe/graphs/acu_face/calculators:tflite_tensors_to_adjust_ear_observer",
    ],
)
mediapipe_simple_subgraph(
    name = "ear_landmark_gpu",
    graph = "ear_landmark_gpu.pbtxt",
    register_as = "EarLandmarkSubgraph",
    deps = [
        ":make_sure_ear_left",
        ":stage_one_ear_landmarkd_detection",

        "//mediapipe/calculators/image:image_cropping_calculator",
        "//mediapipe/calculators/image:image_transformation_calculator",
        "//mediapipe/calculators/tflite:tflite_converter_calculator",
        "//mediapipe/calculators/tflite:tflite_custom_op_resolver_calculator",
        "//mediapipe/calculators/tflite:tflite_inference_calculator",
        "//mediapipe/calculators/tflite:tflite_tensors_to_landmarks_calculator",
        "//mediapipe/calculators/util:landmark_projection_calculator",
        "//mediapipe/graphs/acu_face/calculators:ear_landmarks_to_rects_calculator",
        "//mediapipe/calculators/util:rect_transformation_calculator",
        "//mediapipe/gpu:gpu_buffer_to_image_frame_calculator",
        "//mediapipe/gpu:image_frame_to_gpu_buffer_calculator",
        "//mediapipe/graphs/acu_face/calculators:save_cpu_image_calculator",
        "//mediapipe/graphs/acu_face/calculators:fine_crop_ear_cpu_calculator",
        "//mediapipe/graphs/acu_face/calculators:ear_normalize_calculator",
    ],
)
mediapipe_simple_subgraph(
    name = "multi_ear_landmarks_gpu",
    graph = "multi_ear_landmarks_gpu.pbtxt",
    register_as = "MultiEarLandmarksSubgraph",
    deps = [
        ":ear_landmark_gpu",
        "//mediapipe/calculators/core:begin_loop_calculator",
        "//mediapipe/calculators/core:end_loop_calculator",
        "//mediapipe/calculators/util:filter_collection_calculator",
    ],
)