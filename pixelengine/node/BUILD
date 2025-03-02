load("//:tools.bzl", "default_opts")

cc_library(
    name="node",
    hdrs=glob(["*.h"]),
    visibility=["//visibility:public"],
    copts = default_opts(),
    deps = [
        "//pixelengine/utility"
    ]
)