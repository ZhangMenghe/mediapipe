# Description:
#   a lightweight portable rapid development library for managing OpenGL display / interaction and abstracting video input.

licenses(["notice"])  # BSD license

exports_files(["LICENSE"])

cc_library(
    name = "pangolin",
    srcs = glob(
        [
            "build/src/libpangolin.so"
        ],
    ),
    hdrs = glob(
        [
            "build/src/include/pangolin/*.h",
            "include/pangolin/*/*.h",
            "include/pangolin/*.h",
            "include/**/*.h",
            "include/**/*.hpp"
        ]
    ),
    includes = ["include","build/src/include"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
)
