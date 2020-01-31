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
    srcs = glob(["Thirdparty/g2o/g2o/**/*.cpp",
            "Thirdparty/g2o/g2o/*/*.cpp",
        "Thirdparty/g2o/*.cpp",]),
    hdrs= glob([
        "Thirdparty/g2o/g2o/**/*.h",
        "Thirdparty/g2o/g2o/*/*.h",
        "Thirdparty/g2o/g2o/*/*.hpp",
        "Thirdparty/g2o/*.h",
    ]),
includes=[
    "include"
    ],
deps = [
    "@eigen",
],
    visibility = ["//visibility:public"],
)
cc_library(
    name = "G2O_old",
    srcs = [
        "Thirdparty/g2o/lib/libg2o.so",
        # "Thirdparty/g2o/lib/libg2o_types_sba.so",
        # "Thirdparty/g2o/lib/libg2o_solver_structure_only.so",
        # "Thirdparty/g2o/lib/libg2o_solver_slam2d_linear.so",
        # "Thirdparty/g2o/lib/libg2o_types_icp.so",
        # "Thirdparty/g2o/lib/libg2o_types_slam2d_addons.so",
        # "Thirdparty/g2o/lib/libg2o_interface.so",
        # "Thirdparty/g2o/lib/libg2o_solver_eigen.so",
        # "Thirdparty/g2o/lib/libg2o_tutorial_slam2d.so",
        # "Thirdparty/g2o/lib/libg2o_calibration_odom_laser.so",
        # "Thirdparty/g2o/lib/libg2o_core.so",
        # "Thirdparty/g2o/lib/libg2o_cli.so",
        # "Thirdparty/g2o/lib/libg2o_types_slam3d_addons.so",
        # "Thirdparty/g2o/lib/libg2o_stuff.so",
        # "Thirdparty/g2o/lib/libg2o_types_data.so",
        # "Thirdparty/g2o/lib/libg2o_ext_csparse.so",
        # "Thirdparty/g2o/lib/libg2o_ext_freeglut_minimal.so",
        # "Thirdparty/g2o/lib/libg2o_hierarchical.so",
        # "Thirdparty/g2o/lib/libg2o_solver_dense.so",
        # "Thirdparty/g2o/lib/libg2o_types_slam3d.so",
        # "Thirdparty/g2o/lib/libg2o_csparse_extension.so",
        # "Thirdparty/g2o/lib/libg2o_types_sclam2d.so",
        # "Thirdparty/g2o/lib/libg2o_opengl_helper.so",
        # "Thirdparty/g2o/lib/libg2o_types_sim3.so",
        # "Thirdparty/g2o/lib/libg2o_types_slam2d.so",
        # "Thirdparty/g2o/lib/libg2o_solver_pcg.so",
        # "Thirdparty/g2o/lib/libg2o_solver_csparse.so",
        # "Thirdparty/g2o/lib/libg2o_parser.so",
        # "Thirdparty/g2o/lib/libg2o_simulator.so",
        ],
    hdrs= glob([
        "Thirdparty/g2o/build/g2o/*.h",
        "Thirdparty/g2o/*.h",
        "Thirdparty/g2o/g2o/*/*.h",
        "Thirdparty/g2o/g2o/**/*.h",
        "Thirdparty/g2o/g2o/**/*.hpp",
        "Thirdparty/g2o/g2o/*/*.hpp",        
        ]),
    includes=[
        "Thirdparty/g2o/build",
        "Thirdparty/g2o"
        ],
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
        # "Thirdparty/g2o/g2o/**/*.h",
        # "Thirdparty/g2o/g2o/*/*.h",
        # "Thirdparty/g2o/g2o/*/*.hpp",
        # "Thirdparty/g2o/*.h",
        ]),
    includes=[
        "include"
        ],
    deps = [
        ":DBoW",
        "@pangolin",
        ":G2O",
        "@linux_opengl//:opengl",
        "@linux_opencv//:opencv_contrib",
        "@eigen",
    ],
    visibility = ["//visibility:public"],
    alwayslink = 1,
)