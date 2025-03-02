load("//:tools.bzl", "default_opts")

cc_library(
    name="metallib",
    hdrs=glob(["metal-cpp/**/*.hpp", "metal-cpp-extensions/**/*.hpp"]),
    srcs=['definition.cpp', "MetalImpl.cpp"],
    includes=['metal-cpp', 'metal-cpp-extensions'],
    visibility=["//visibility:public"],
    copts = default_opts(),
    linkopts = [
        "-framework Metal",
        "-framework MetalKit",
        "-framework AppKit",
        "-framework Foundation",
        "-framework QuartzCore",
        "-framework CoreGraphics",
    ]
)