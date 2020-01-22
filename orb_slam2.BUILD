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
        # "lib/libORB_SLAM2.so",
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
        "Thirdparty/g2o/g2o/*/*.h",
        "Thirdparty/g2o/g2o/*/*.hpp",
        "Thirdparty/g2o/*.h",
        ]),
    includes=[
        "include"
        ],
    deps = [
        "@linux_opencv//:opencv_contrib",
        ":G2O",
        ":DBoW",
        "@pangolin",
        "@linux_opengl//:opengl",
        "@eigen",
    ],
    visibility = ["//visibility:public"],
    alwayslink = 1,
)