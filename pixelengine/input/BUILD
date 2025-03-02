load("//:tools.bzl", "default_opts")

cc_library(
    name="input",
    hdrs=glob(["*.h"]),
    srcs=glob(["*.cpp"]),
    deps=[
        "//pixelengine/utility"
    ],
    visibility=["//visibility:public"],
    copts = default_opts(),
)