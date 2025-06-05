cc_test(
    name = "eesi_service_test",
    size = "small",
    srcs = [
        "eesi_service_test.cc",
        "error_blocks_helper.cc",
        "error_blocks_helper.h",
        "mock_synonym_finder.h",
    ],
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//:testdata_bitcode",
    ],
    flaky = 1,
    includes = ["include"],
    deps = [
        "//bitcode:service",
        "//common:servers",
        "//eesi:service",
        "//proto:bitcode_cc_grpc",
        "//proto:eesi_cc_grpc",
        "//proto:operations_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@gtest//:main",
    ],
)

cc_test(
    name = "error_blocks_test",
    size = "small",
    srcs = [
        "error_blocks_helper.cc",
        "error_blocks_helper.h",
        "error_blocks_test.cc",
        "mock_synonym_finder.h",
    ],
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//:testdata_bitcode",
    ],
    includes = ["include"],
    deps = [
        "//eesi:eesi_llvm_passes",
        "//eesi:service",
        "//proto:eesi_cc_grpc",
        "@gtest//:main",
    ],
)

cc_test(
    name = "error_blocks_with_violations_test",
    size = "small",
    srcs = [
        "error_blocks_helper.cc",
        "error_blocks_helper.h",
        "error_blocks_with_violations_test.cc",
        "mock_synonym_finder.h",
    ],
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//:testdata_bitcode",
    ],
    includes = ["include"],
    deps = [
        "//eesi:eesi_llvm_passes",
        "//eesi:service",
        "//proto:eesi_cc_grpc",
        "@gtest//:main",
    ],
)

cc_test(
    name = "error_blocks_non_doomed_functions_test",
    size = "small",
    srcs = [
        "error_blocks_helper.cc",
        "error_blocks_helper.h",
        "error_blocks_non_doomed_functions_test.cc",
        "mock_synonym_finder.h",
    ],
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//:testdata_bitcode",
    ],
    includes = ["include"],
    deps = [
        "//eesi:eesi_llvm_passes",
        "//eesi:service",
        "//proto:eesi_cc_grpc",
        "@gtest//:main",
    ],
)

cc_test(
    name = "return_constraints_test",
    size = "small",
    srcs = ["return_constraints_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//:testdata_bitcode",
    ],
    includes = ["include"],
    deps = [
        "//eesi:eesi_llvm_passes",
        "//eesi:service",
        "//proto:eesi_cc_grpc",
        "@gtest//:main",
    ],
)

cc_test(
    name = "lattice_test",
    size = "small",
    srcs = ["lattice_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    includes = ["include"],
    deps = [
        "//eesi:service",
        "@gtest//:main",
    ],
)

cc_test(
    name = "confidence_lattice_test",
    size = "small",
    srcs = ["confidence_lattice_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    includes = ["include"],
    deps = [
        "//eesi:service",
        "@gtest//:main",
    ],
)

cc_test(
    name = "abstract_icmp_test",
    size = "small",
    srcs = ["abstract_icmp_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    includes = ["include"],
    deps = [
        "//eesi:service",
        "@gtest//:main",
    ],
)

cc_test(
    name = "return_range_test",
    size = "small",
    srcs = ["return_range_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    data = [
        "//:testdata_bitcode",
    ],
    includes = ["include"],
    deps = [
        "//eesi:service",
        "@gtest//:main",
    ],
)

cc_test(
    name = "eesi_common_test",
    size = "small",
    srcs = ["eesi_common_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    includes = ["include"],
    deps = [
        "//eesi:service",
        "@gtest//:main",
    ],
)
