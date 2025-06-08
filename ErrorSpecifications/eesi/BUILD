cc_library(
    name = "eesi_llvm_passes",
    srcs = [
        "include/call_graph_underapproximation.h",
        "include/checker.h",
        "include/confidence_lattice.h",
        "include/constraint.h",
        "include/eesi_common.h",
        "include/error_blocks_pass.h",
        "include/return_constraints_pass.h",
        "include/return_propagation_pass.h",
        "include/return_range_pass.h",
        "include/returned_values_pass.h",
        "include/gpt_model.h",
        "src/call_graph_underapproximation.cc",
        "src/checker.cc",
        "src/confidence_lattice.cc",
        "src/constraint.cc",
        "src/eesi_common.cc",
        "src/error_blocks_pass.cc",
        "src/return_constraints_pass.cc",
        "src/return_propagation_pass.cc",
        "src/return_range_pass.cc",
        "src/returned_values_pass.cc",
        "src/gpt_model.cc",
    ],
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        "//common:llvm",
        "//proto:eesi_cc_grpc",
        "//proto:gpt_cc_grpc",
        "@com_github_01org_tbb//:tbb",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "service",
    srcs = [
        "src/eesi_server.cc",
    ],
    hdrs = [
        "include/eesi_server.h",
    ],
    includes = ["include"],
    deps = [
        ":eesi_llvm_passes",
        "//common:llvm",
        "//common:operations",
        "//common:servers",
        "//proto:eesi_cc_grpc",
        "@com_github_01org_tbb//:tbb",
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
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
)
