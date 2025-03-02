load("//:tools.bzl", "default_opts")

cc_library(
    name="world",
    hdrs=glob(["*.h"]),
    srcs=glob(["*.cpp"]),
    deps=[
        "//pixelengine/node",
        "//pixelengine/graphics"
    ],
    visibility=["//visibility:public"],
    copts = default_opts()
)