# Description:
#   OpenCV libraries for video/image processing on Linux

licenses(["notice"])  # BSD license

exports_files(["LICENSE"])

cc_library(
        name = "pthread",
        srcs = glob(
            [
                "lib/x86_64-linux-gnu/libpthread.so",
            ],
        ),
        #hdrs = glob(["include/GL/*.h*"]),
        #includes = ["include"],
        linkstatic = 1,
        visibility = ["//visibility:public"],
    )