licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

exports_files(["LICENSE"])

cc_library(
    name = "ORB_SLAM2",
    srcs = ["src/TEST_BAZEL.cc"],
    hdrs= glob(["include/*.h"]),
    includes=[
        "include"
        
        ],
    deps = [
        "@eigen"
    ],
    visibility = ["//visibility:public"],
    alwayslink = 1,
)