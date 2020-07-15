licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

exports_files(["LICENSE"])


#vrNative
cc_library(
    name = "vrNative",
    srcs = [
        "platforms/desktop/utils/dicomLoader.cpp",
        "platforms/desktop/utils/uiController.cpp",
        "vrController.cpp",
        "overlayController.cpp",
        "Manager.cpp",
        "dicomRenderer/raycastRenderer.cpp",
        "dicomRenderer/texturebasedRenderer.cpp",
        "dicomRenderer/cuttingController.cpp",
        "dicomRenderer/colorbarRenderer.cpp",
        "dicomRenderer/graphRenderer.cpp",
        "dicomRenderer/screenQuad.cpp",
        "GLPipeline/Shader.cpp",
        "GLPipeline/Mesh.cpp",
        "GLPipeline/Texture.cpp",
    ],
    hdrs = glob(
        [
            "*.h",
            "GLPipeline/*.h",
            "dicomRenderer/*.h",
            "Utils/*.h",
            "platforms/*.h",
            "platforms/desktop/*.h",
            "platforms/desktop/utils/*.h",
        ]
    ),
    includes=[
        "./",
        "platforms/desktop",
        "platforms/desktop/utils/*.h",
    ],
    defines = ["MEDIAPIPE"],
    deps = [
        "@linux_opengl//:opengl",
        "@linux_usr//:glm",
    ],
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
    # defines = ["ASSET_PATH=\"../app/src/main/assets/\""],
    visibility = ["//visibility:public"],
    alwayslink = 1,
)