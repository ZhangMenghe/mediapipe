licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:private"])

exports_files(["LICENSE"])

#DBoW
cc_library(
    name = "DBoW",
    srcs = ["Thirdparty/DBoW2/lib/libDBoW2.so"],
    visibility = ["//visibility:public"],
)
#G2O
cc_library(
    name = "G2O",
    srcs = glob([
        "Thirdparty/g2o/g2o/**/*.cpp",
        "Thirdparty/g2o/g2o/*/*.cpp",
        "Thirdparty/g2o/*.cpp",
    ]),
    hdrs = glob([
        "Thirdparty/g2o/g2o/**/*.h",
        "Thirdparty/g2o/g2o/*/*.h",
        "Thirdparty/g2o/g2o/*/*.hpp",
        "Thirdparty/g2o/*.h",
    ]),
    includes=["include"],
    deps = [
        "@linux_usr//:eigen",
    ],
    visibility = ["//visibility:public"],
)

#pangolin
cc_library(
    name = "pangolin",
    srcs = glob(
        [
            "Thirdparty/Pangolin/build/src/libpangolin.so"
        ],
    ),
    hdrs = glob(
        [
            "Thirdparty/Pangolin/build/src/include/pangolin/*.h",
            "Thirdparty/Pangolin/include/pangolin/*/*.h",
            "Thirdparty/Pangolin/include/pangolin/*.h",
            "Thirdparty/Pangolin/include/**/*.h",
            "Thirdparty/Pangolin/include/**/*.hpp"
        ]
    ),
    deps = ["@linux_opengl//:opengl",],
    includes = ["Thirdparty/Pangolin/include","Thirdparty/Pangolin/build/src/include"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "ORB_SLAM2",
    srcs = [
        "src/System.cc",
        "src/Tracking.cc",
        "src/LocalMapping.cc",
        "src/LoopClosing.cc",
        "src/ORBextractor.cc",
        "src/ORBmatcher.cc",
        "src/FrameDrawer.cc",
        "src/Converter.cc",
        "src/MapPoint.cc",
        "src/KeyFrame.cc",
        "src/Map.cc",
        # "src/MapDrawer.cc",
        "src/Optimizer.cc",
        "src/PnPsolver.cc",
        "src/Frame.cc",
        "src/KeyFrameDatabase.cc",
        "src/Sim3Solver.cc",
        "src/Initializer.cc",
        "src/Viewer.cc",
        ],
    hdrs= glob([
        "include/*.h",
        "Thirdparty/DBoW2/DBoW2/*.h",
        "Thirdparty/DBoW2/DUtils/*.h",
        ]),
    includes=[
        "include"
        ],
    deps = [
        ":DBoW",
        ":pangolin",
        ":G2O",
        "@linux_opengl//:opengl",
        "@linux_opencv//:opencv",
        "@linux_opencv//:opencv_contrib",
        "@linux_usr//:eigen",
    ],
    visibility = ["//visibility:public"],
    alwayslink = 1,
)