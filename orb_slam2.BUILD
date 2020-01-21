licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

exports_files(["LICENSE"])
cc_library(
    name = "DBoW",
    srcs = ["Thirdparty/DBoW2/lib/libDBoW2.so"],
    visibility = ["//visibility:public"],
)
cc_library(
    name = "G2O",
    srcs = ["Thirdparty/g2o/lib/libg2o.so"],
    visibility = ["//visibility:public"],
)
cc_library(
    name = "ORB_SLAM2",
    srcs = [
        "lib/libORB_SLAM2.so"
        ],
    hdrs= glob([
        "include/*.h",
        "Thirdparty/DBoW2/DBoW2/*.h",
        "Thirdparty/DBoW2/DUtils/*.h",
        "Thirdparty/g2o/g2o/*/*.h",
        "Thirdparty/g2o/g2o/*/*.hpp",
        "Thirdparty/g2o/*.h",
        ]),
    includes=[
        "include"
        ],
    deps = [
        #":G2O",
        #":DBoW",
        #"@pangolin",
        "@eigen",
    ],
    visibility = ["//visibility:public"],
    alwayslink = 1,
)