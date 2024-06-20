cc_test(
    name = "defined_functions_test",
    size = "small",
    srcs = ["defined_functions_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//:testdata_bitcode",
    ],
    deps = [
        "//bitcode:defined_functions_pass",
        "//proto:bitcode_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@gtest//:main",
        "@org_llvm//:LLVMIRReader",
    ],
)

cc_test(
    name = "called_functions_test",
    size = "small",
    srcs = [
        "called_functions_helper.cc",
        "called_functions_helper.h",
        "called_functions_test.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//:testdata_bitcode",
    ],
    deps = [
        "//bitcode:called_functions_pass",
        "//proto:bitcode_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@gtest//:main",
        "@org_llvm//:LLVMIRReader",
    ],
)

cc_test(
    name = "local_called_functions_test",
    size = "small",
    srcs = [
        "called_functions_helper.cc",
        "called_functions_helper.h",
        "local_called_functions_test.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//:testdata_bitcode",
    ],
    deps = [
        "//bitcode:called_functions_pass",
        "//bitcode:local_called_functions_pass",
        "//proto:bitcode_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@gtest//:main",
        "@org_llvm//:LLVMIRReader",
    ],
)

cc_test(
    name = "file_called_functions_test",
    size = "small",
    srcs = [
        "called_functions_helper.cc",
        "called_functions_helper.h",
        "file_called_functions_test.cc",
    ],
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//:testdata_bitcode",
    ],
    deps = [
        "//bitcode:called_functions_pass",
        "//bitcode:file_called_functions_pass",
        "//proto:bitcode_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@gtest//:main",
        "@org_llvm//:LLVMIRReader",
    ],
)

cc_test(
    name = "bitcode_service_test",
    size = "small",
    srcs = ["bitcode_service_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//:testdata_bitcode",
    ],
    deps = [
        "//bitcode:service",
        "//common:servers",
        "//proto:bitcode_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@gtest//:main",
    ],
)

py_binary(
    name = "test_client",
    srcs = ["test_client.py"],
    deps = [
        "//proto:bitcode_py_grpc",
        "//proto:operations_py_grpc",
        "//proto:status_py_grpc",
    ],
)
