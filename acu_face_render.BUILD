licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

exports_files(["LICENSE"])

cc_library(
    name = "acuRenderer",
    srcs = [
        "PrimeRenderer/PointCloudRenderer.cpp",
        "GLPipeline/Shader.cpp",
        "GLPipeline/Mesh.cpp",
        "GLPipeline/Texture.cpp",
    ],
    hdrs = glob(
        [
            "*.h",
            "PrimeRenderer/*.h",
            "GLPipeline/*.h",
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
        # "@linux_opengl//:opengl",
        "@linux_usr//:glm",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
)
