licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

exports_files(["LICENSE"])


#vrNative
cc_library(
    name = "vrNative",
    srcs = glob(
        [
            "build/libvrNative.so"
        ],
    ),
    hdrs = glob(
        [
            "*.h",
            "GLPipeline/*.h",
            "dicomRenderer/*.h",
            "Utils/*.h",
            "platforms/*.h",
        ]
    ),
    includes=[
        "./"
    ],
    deps = ["@linux_opengl//:opengl",],
    linkstatic = 1,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "HELMSLEY_VR",
    srcs = [
        "platforms/desktop/utils/dicomLoader.cpp",
        "platforms/desktop/utils/uiController.cpp",

        # "platforms/desktop/RPCs/proto/transManager.grpc.pb.cc"
        # "platforms/desktop/RPCs/proto/inspectorSync.grpc.pb.cc"
        # "platforms/desktop/RPCs/proto/common.grpc.pb.cc"

        # "platforms/desktop/RPCs/proto/transManager.pb.cc"
        # "platforms/desktop/RPCs/proto/inspectorSync.pb.cc"
        # "platforms/desktop/RPCs/proto/common.pb.cc"
        ],
    hdrs= glob([
        "platforms/desktop/*.h",
        "platforms/desktop/utils/*.h",
        ]),
    includes=[
        "platforms/desktop",
        "platforms/desktop/utils",
        "./"
        ],
    deps = [
        ":vrNative",
        "@linux_opengl//:opengl",
        "@linux_usr//:glm",
    ],
    visibility = ["//visibility:public"],
    alwayslink = 1,
)