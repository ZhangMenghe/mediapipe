# Description:
#   OpenCV libraries for video/image processing on Linux

licenses(["notice"])  # BSD license

exports_files(["LICENSE"])

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

# # Description:
# #   Eigen is a C++ template library for linear algebra: vectors,
# #   matrices, and related algorithms.

# licenses([
#     # Note: Eigen is an MPL2 library that includes GPL v3 and LGPL v2.1+ code.
#     #       We've taken special care to not reference any restricted code.
#     "reciprocal",  # MPL2
#     "notice",  # Portions BSD
# ])

# exports_files(["COPYING.MPL2"])

# EIGEN_FILES = [
#     "Eigen/**",
#     "unsupported/Eigen/CXX11/**",
#     "unsupported/Eigen/FFT",
#     "unsupported/Eigen/KroneckerProduct",
#     "unsupported/Eigen/src/FFT/**",
#     "unsupported/Eigen/src/KroneckerProduct/**",
#     "unsupported/Eigen/MatrixFunctions",
#     "unsupported/Eigen/SpecialFunctions",
#     "unsupported/Eigen/src/MatrixFunctions/**",
#     "unsupported/Eigen/src/SpecialFunctions/**",
# ]

# # Files known to be under MPL2 license.
# EIGEN_MPL2_HEADER_FILES = glob(
#     EIGEN_FILES,
#     exclude = [
#         # Guarantees that any non-MPL2 file added to the list above will fail to
#         # compile.
#         #"Eigen/src/Core/util/NonMPL2.h",
#         "Eigen/**/CMakeLists.txt",
#     ],
# )

# cc_library(
#     name = "eigen",
#     hdrs = EIGEN_MPL2_HEADER_FILES,
#     defines = [
#         # This define (mostly) guarantees we don't link any problematic
#         # code. We use it, but we do not rely on it, as evidenced above.
#         #"EIGEN_MPL2_ONLY",
#         "EIGEN_MAX_ALIGN_BYTES=64",
#         "EIGEN_HAS_TYPE_TRAITS=0",
#     ],
#     includes = ["."],
#     visibility = ["//visibility:public"],
# )

# filegroup(
#     name = "eigen_header_files",
#     srcs = EIGEN_MPL2_HEADER_FILES,
#     visibility = ["//visibility:public"],
# )