licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

exports_files(["LICENSE"])

load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")


proto_library(
    name = "helm_proto",
    srcs = [
        "proto/common.proto",
        "proto/transManager.proto",
        "proto/inspectorSync.proto",
        ],
)

cc_proto_library(
    name = "helm_cc_proto",
    deps = [":helm_proto"],
)

cc_grpc_library(
    name = "helm_cc_grpc",
    srcs = [":helm_proto"],
    grpc_only = True,
    deps = [":helm_cc_proto"],
)

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
    ]+ select({
            ":rpc_disabled": [],
            "//conditions:default": [
                "platforms/desktop/RPCs/rpcHandler.cpp",
                # "platforms/desktop/RPCs/proto/transManager.grpc.pb.cc",
                # "platforms/desktop/RPCs/proto/inspectorSync.grpc.pb.cc",
                # "platforms/desktop/RPCs/proto/common.grpc.pb.cc",

                # "platforms/desktop/RPCs/proto/transManager.pb.cc",
                # "platforms/desktop/RPCs/proto/inspectorSync.pb.cc",
                # "platforms/desktop/RPCs/proto/common.pb.cc",
        ],
    }),
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
    )+ select({
            ":rpc_disabled": [],
            "//conditions:default": 
                glob([
                # "platforms/desktop/RPCs/proto/*.grpc.pb.h",
                # "platforms/desktop/RPCs/proto/*.pb.h",
                # "platforms/desktop/RPCs/proto/*.h",
                "platforms/desktop/RPCs/*.h",]
                )
                
    }),
    includes=[
        "./",
        "platforms/desktop",
        "platforms/desktop/utils/*.h",
    ],
    # + select({
    #         ":rpc_disabled": [],
    #         "//conditions:default": [
    #             "platforms/desktop/RPCs"
    #     ],
    # }),
    defines = ["MEDIAPIPE"]+ select({
            ":rpc_disabled": [],
            "//conditions:default": [
                "RPC_ENABLED"
        ],
    }),
    deps = [
        ":helm_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@linux_opengl//:opengl",
        "@linux_usr//:glm",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
)
config_setting(
    name = "rpc_disabled",
    define_values = {
        "RPC_ENABLED": "0",
    },
    visibility = ["//visibility:public"],
)
