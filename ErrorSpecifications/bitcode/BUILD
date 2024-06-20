cc_library(
    name = "annotate_pass",
    srcs = [
        "src/annotate_pass.cc",
        "src/annotate_pass.h",
    ],
    visibility = ["//bitcode/test:__pkg__"],
    deps = [
        "//common:llvm",
        "//proto:bitcode_cc_grpc",
    ],
)

cc_library(
    name = "defined_functions_pass",
    srcs = [
        "src/defined_functions_pass.cc",
        "src/defined_functions_pass.h",
    ],
    visibility = ["//bitcode/test:__pkg__"],
    deps = [
        "//common:llvm",
        "//proto:bitcode_cc_grpc",
    ],
)

cc_library(
    name = "called_functions_pass",
    srcs = [
        "src/called_functions_pass.cc",
        "src/called_functions_pass.h",
    ],
    visibility = ["//bitcode/test:__pkg__"],
    deps = [
        "//common:llvm",
        "//proto:bitcode_cc_grpc",
    ],
)

cc_library(
    name = "local_called_functions_pass",
    srcs = [
        "src/local_called_functions_pass.cc",
        "src/local_called_functions_pass.h",
    ],
    visibility = ["//bitcode/test:__pkg__"],
    deps = [
        "//common:llvm",
        "//proto:bitcode_cc_grpc",
    ],
)

cc_library(
    name = "file_called_functions_pass",
    srcs = [
        "src/file_called_functions_pass.cc",
        "src/file_called_functions_pass.h",
    ],
    visibility = ["//bitcode/test:__pkg__"],
    deps = [
        "//common:llvm",
        "//proto:bitcode_cc_grpc",
    ],
)

cc_library(
    name = "service",
    srcs = [
        "src/bitcode_server.cc",
    ],
    hdrs = [
        "include/bitcode_server.h",
    ],
    includes = ["include"],
    visibility = [
        "//bitcode/test:__pkg__",
        "//checker/test:__pkg__",
        "//eesi/test:__pkg__",
        "//getgraph/test:__pkg__",
    ],
    deps = [
        ":annotate_pass",
        ":called_functions_pass",
        ":defined_functions_pass",
        ":file_called_functions_pass",
        ":local_called_functions_pass",
        "//common:operations",
        "//common:servers",
        "//proto:bitcode_cc_grpc",
        "@com_github_01org_tbb//:tbb",
        "@com_github_grpc_grpc//:grpc++",
        "@org_llvm//:LLVMAnalysis",
        "@org_llvm//:LLVMBitWriter",
        "@org_llvm//:LLVMCore",
        "@org_llvm//:LLVMIRReader",
        "@org_llvm//:LLVMSupport",
    ],
)

cc_binary(
    name = "main",
    srcs = [
        "src/main.cc",
    ],
    visibility = ["//cli/test/common:__pkg__"],
    deps = [
        ":service",
        "//common:servers",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
)
