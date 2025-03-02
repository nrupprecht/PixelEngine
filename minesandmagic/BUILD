load("//:tools.bzl", "default_opts")

cc_library(
    name="minesandmagic",
    hdrs=glob(["**/*.h"]),
    srcs=glob(["**/*.cpp"]),
    visibility=["//visibility:public"],
    deps=["//pixelengine/application", "//pixelengine/physics"],
    copts = default_opts(),
)