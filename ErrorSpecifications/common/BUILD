cc_library(
    name = "llvm",
    srcs = [
        "src/llvm.cc",
    ],
    hdrs = [
        "include/llvm.h",
    ],
    includes = ["include"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "servers",
        "//proto:bitcode_cc_grpc",
        "@com_github_01org_tbb//:tbb",
        "@com_github_google_glog//:glog",
        "@org_llvm//:LLVMAnalysis",
        "@org_llvm//:LLVMCore",
        "@org_llvm//:LLVMSupport",
    ],
)

cc_library(
    name = "operations",
    srcs = [
        "src/operations_service.cc",
    ],
    hdrs = [
        "include/operations_service.h",
    ],
    includes = ["include"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "//proto:bitcode_cc_grpc",
        "//proto:operations_cc_grpc",
        "@com_github_01org_tbb//:tbb",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "servers",
    srcs = [
        "src/servers.cc",
    ],
    hdrs = [
        "include/servers.h",
    ],
    includes = ["include"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "//proto:operations_cc_grpc",
        "@com_github_google_glog//:glog",
        "@com_github_googleapis_google_cloud_cpp//google/cloud/storage:storage_client",
        "@com_github_grpc_grpc//:grpc++",
    ],
)
