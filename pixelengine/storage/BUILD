load("//:tools.bzl", "default_opts")

cc_library(
    name="storage",
    hdrs=glob(["*.h"]),
    srcs=glob(["*.cpp"]),
    deps=[
        "//pixelengine/graphics",
        "//pixelengine/utility",
    ],
    visibility=["//visibility:public"],
    copts = default_opts(),
)