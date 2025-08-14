load("//:tools.bzl", "default_opts")

cc_library(
    name="utility",
    hdrs=glob(["**/*.h"]),
    srcs=glob(["**/*.cpp"]),
    deps=[
        "@lightning//:lightning",
        "//metallib"
    ],
    visibility=["//visibility:public"],
    copts = default_opts(),
)