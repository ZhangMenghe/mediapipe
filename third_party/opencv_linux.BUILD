# Description:
#   OpenCV libraries for video/image processing on Linux

licenses(["notice"])  # BSD license

exports_files(["LICENSE"])

# The following build rule assumes that OpenCV is installed by
# 'apt-get install libopencv-core-dev libopencv-highgui-dev \'
# '                libopencv-calib3d-dev libopencv-features2d-dev \'
# '                libopencv-imgproc-dev libopencv-video-dev'
# on Debian buster/Ubuntu 18.04.
# If you install OpenCV separately, please modify the build rule accordingly.
# cc_library(
#    name = "opencv",
#    srcs = glob(
#        [
#            "lib/x86_64-linux-gnu/libopencv_core.so",
#            "lib/x86_64-linux-gnu/libopencv_highgui.so",
#            "lib/x86_64-linux-gnu/libopencv_imgcodecs.so",
#            "lib/x86_64-linux-gnu/libopencv_imgproc.so",
#            "lib/x86_64-linux-gnu/libopencv_video.so",
#            "lib/x86_64-linux-gnu/libopencv_videoio.so",
#        ],
#     ),
#     hdrs = glob(["include/opencv2/**/*.h*"]),
#     includes = ["include"],
#     linkstatic = 1,
#     visibility = ["//visibility:public"],
# )
cc_library(
    name = "opencv",
    srcs = glob(
        [
            "lib/libopencv_core.so.4.5",
            "lib/libopencv_calib3d.so.4.5",
            "lib/libopencv_features2d.so.4.5",
            "lib/libopencv_highgui.so.4.5",
            "lib/libopencv_imgcodecs.so.4.5",
            "lib/libopencv_imgproc.so.4.5",
            "lib/libopencv_video.so.4.5",
            "lib/libopencv_videoio.so.4.5",
        ],
    ),
    hdrs = glob(["include/opencv2/**/*.h*"]),
    includes = ["include/"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
)
cc_library(
        name = "opencv_contrib",
        srcs = glob(
            [
                "lib/libopencv_calib3d.so.4.5",
                "lib/libopencv_flann.so.4.5",
                "lib/libopencv_xfeatures2d.so.4.5",
                "lib/libopencv_features2d.so.4.5",
            ],
        ),
        hdrs = glob(["include/opencv4/opencv2/**/*.h*"]),
        includes = ["include/opencv4"],
        linkstatic = 1,
        visibility = ["//visibility:public"],
)