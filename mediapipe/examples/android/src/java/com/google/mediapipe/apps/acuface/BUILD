# Copyright 2019 The MediaPipe Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

cc_binary(
    name = "libmediapipe_jni.so",
    linkshared = 1,
    linkstatic = 1,
    deps = [
        "//mediapipe/graphs/acu_face:mobile_calculators",
        "//mediapipe/java/com/google/mediapipe/framework/jni:acu_jni",
        "//mediapipe/java/com/google/mediapipe/framework/jni:mediapipe_framework_jni",
    ],
)

cc_library(
    name = "mediapipe_jni_lib",
    srcs = [":libmediapipe_jni.so"],
    alwayslink = 1,
)

#android_library(
#    name = "acu_jni_core",
#    srcs = ["JNIInterface.java"],
#    proguard_specs = [":proguard.pgcfg"],
#    visibility = ["//visibility:public"],
#)
#
#android_library(
#    name = "acu_jni_framework",
#    proguard_specs = [":proguard.pgcfg"],
#    visibility = ["//visibility:public"],
#    exports = [
#        ":acu_jni_core",
#    ],
#)

android_binary(
    name = "acufacegpu",
    srcs = glob(["*.java"]),
    assets = [
        "//mediapipe/graphs/acu_face:mobile_gpu.binarypb",
        "//mediapipe/models:hair_segmentation.tflite",
        "//mediapipe/modules/face_landmark:face_landmark.tflite",
        "//mediapipe/modules/face_detection:face_detection_front.tflite",
        "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/acuface/assets:acu.csv",
    ],
    assets_dir = "",
    manifest = "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/basic:AndroidManifest.xml",
    manifest_values = {
        "applicationId": "com.google.mediapipe.apps.acufacegpu",
        "appName": "FaceAtlasAR",
        "mainActivity": "com.google.mediapipe.apps.acuface.MainActivity",
        "cameraFacingFront": "True",
        "binaryGraphName": "mobile_gpu.binarypb",
        "inputVideoStreamName": "input_video",
        "outputVideoStreamName": "output_video",
        "flipFramesVertically": "True",
    },
    multidex = "native",
    resource_files = glob(["res/**"]),
    deps = [
        ":mediapipe_jni_lib",
        "//mediapipe/examples/android/src/java/com/google/mediapipe/apps/basic:basic_lib",
        "//mediapipe/framework/formats:landmark_java_proto_lite",
        "//mediapipe/java/com/google/mediapipe/components:android_camerax_helper",
        "//mediapipe/java/com/google/mediapipe/components:android_components",
        "//mediapipe/java/com/google/mediapipe/framework:android_framework",
        "//mediapipe/java/com/google/mediapipe/glutil",
        "//third_party:androidx_appcompat",
        "//third_party:androidx_constraint_layout",
        "//third_party:androidx_core",
        "//third_party:androidx_material",
        "//third_party:androidx_recyclerview",
        "//third_party:opencv",
        "@maven//:androidx_concurrent_concurrent_futures",
        "@maven//:com_google_guava_guava",
    ],
)
