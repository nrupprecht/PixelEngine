load("//:tools.bzl", "default_opts")

cc_library(
    name="graphics",
    hdrs=glob(["*.h"]),
    srcs=glob(["*.cpp"]),
    deps=[
        "//pixelengine/node"
    ],
    visibility=["//visibility:public"],
    copts = default_opts(),
)