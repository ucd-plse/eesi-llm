# From https://github.com/rnburn/bazel-cookbook/blob/master/tbb/tbb.BUILD

genrule(
    name = "build_tbb",
    srcs = glob(["**"]) + [
        "@local_config_cc//:toolchain",
    ],
    outs = [
        "libtbb.a",
        "libtbbmalloc.a",
    ],
    cmd = """
         set -e
         WORK_DIR=$$PWD
         DEST_DIR=$$PWD/$(@D)
         cd $$(dirname $(location :Makefile))

         #TBB's build needs some help to figure out what compiler it's using
         COMPILER_OPT="compiler=gcc"

         # uses extra_inc=big_iron.inc to specify that static libraries are
         # built. See https://software.intel.com/en-us/forums/intel-threading-building-blocks/topic/297792
         make tbb_build_prefix="build" \
              extra_inc=big_iron.inc \
              $$COMPILER_OPT; \

         echo cp build/build_{release,debug}/*.a $$DEST_DIR
         cp build/build_{release,debug}/*.a $$DEST_DIR
         cd $$WORK_DIR
  """,
)

cc_library(
    name = "tbb",
    srcs = ["libtbb.a"],
    hdrs = glob([
        "include/serial/**",
        "include/tbb/**/**",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
