load("//:tools.bzl", "default_opts")

cc_library(
    name="application",
    hdrs=glob(["*.h"]),
    srcs=glob(["*.cpp"]),
    deps=[
        "//pixelengine/world",
        "//pixelengine/input",
    ],
    visibility=["//visibility:public"],
    copts = default_opts(),
)