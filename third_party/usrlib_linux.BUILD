# Description:
# Usr library common dependencies

licenses(["notice"])  # BSD license

exports_files(["LICENSE"])

#Eigen
cc_library(
    name = "eigen",
    hdrs = glob([
        "include/eigen3/Eigen/*/*/*.h",
        "include/eigen3/Eigen/*/*/*/*/*.h",
        "include/eigen3/Eigen/*/*/*/*.h",
    ])+[
        "include/eigen3/Eigen/Core",
        "include/eigen3/Eigen/LU",
        "include/eigen3/Eigen/Householder",
        "include/eigen3/Eigen/QtAlignedMalloc",
        "include/eigen3/Eigen/SVD",
        "include/eigen3/Eigen/StdDeque",
        "include/eigen3/Eigen/MetisSupport",
        "include/eigen3/Eigen/Jacobi",
        "include/eigen3/Eigen/SparseCore",
        "include/eigen3/Eigen/UmfPackSupport",
        "include/eigen3/Eigen/StdVector",
        "include/eigen3/Eigen/Cholesky",
        "include/eigen3/Eigen/PaStiXSupport",
        "include/eigen3/Eigen/Geometry",
        "include/eigen3/Eigen/OrderingMethods",
        "include/eigen3/Eigen/QR",
        "include/eigen3/Eigen/Dense",
        "include/eigen3/Eigen/IterativeLinearSolvers",
        "include/eigen3/Eigen/Sparse",
        "include/eigen3/Eigen/PardisoSupport",
        "include/eigen3/Eigen/CholmodSupport",
        "include/eigen3/Eigen/Eigenvalues",
        "include/eigen3/Eigen/SparseCholesky",
        "include/eigen3/Eigen/StdList",
        "include/eigen3/Eigen/SparseLU",
        "include/eigen3/Eigen/SparseQR",
        "include/eigen3/Eigen/Eigen",
        "include/eigen3/Eigen/SuperLUSupport",
        "include/eigen3/Eigen/SPQRSupport",

    ],
    includes = ["include/eigen3",
                "include/eigen3/Eigen"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
)

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