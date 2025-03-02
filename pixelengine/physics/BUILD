load("//:tools.bzl", "default_opts")

cc_library(
    name="physics",
    hdrs=glob(["*.h"]),
    srcs=glob(["*.cpp"]),
    deps=[
        "//pixelengine/world"
    ],
    visibility=["//visibility:public"],
    copts = default_opts(),
)