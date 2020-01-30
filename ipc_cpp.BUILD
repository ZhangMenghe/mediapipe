licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

exports_files(["LICENSE"])
cc_library(
    name = "IPC_CPP",
    srcs = ["output/libipc.so"],
    hdrs= glob([
        "include/*.h",
        "src/*.inc",
        "src/memory/*.hpp",
        "src/platform/*.h",
        "src/memory/*.h",
        "src/circ/*.h",
        ]),
    includes=[
        "include",
        "src",
        "src/memory"
        ],
    deps = [
        "@linux_rt//:rt"
    ],
    visibility = ["//visibility:public"],
)