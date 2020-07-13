licenses(["notice"])

exports_files(["LICENSE"])

#OpenGL
cc_library(
    name = "opengl",
    srcs = glob(
        [
            "lib/x86_64-linux-gnu/libGLU.so",
            "lib/x86_64-linux-gnu/libGLEW.so",
        ],
    ),
    hdrs = glob(["include/GL/*.h*"]),
    includes = ["include"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
)