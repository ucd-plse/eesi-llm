package(
    default_visibility = ["//visibility:public"],
)

load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("@com_github_grpc_grpc//bazel:python_rules.bzl", "py_grpc_library", "py_proto_library")

proto_library(
    name = "bitcode_proto",
    srcs = ["bitcode.proto"],
    deps = [":operations_proto"],
)

cc_proto_library(
    name = "bitcode_cc_proto",
    deps = ["bitcode_proto"],
)

# See https://github.com/grpc/grpc/pull/18955
cc_grpc_library(
    name = "bitcode_cc_grpc",
    srcs = [":bitcode_proto"],
    grpc_only = True,
    deps = [":bitcode_cc_proto"],
)

py_proto_library(
    name = "bitcode_py_proto",
    deps = [":bitcode_proto"],
)

py_grpc_library(
    name = "bitcode_py_grpc",
    srcs = [":bitcode_proto"],
    deps = [":bitcode_py_proto"],
)

proto_library(
    name = "domain_knowledge_proto",
    srcs = ["domain_knowledge.proto"],
    deps = [":bitcode_proto"],
)

cc_proto_library(
    name = "domain_knowledge_cc_proto",
    deps = ["domain_knowledge_proto"],
)

py_proto_library(
    name = "domain_knowledge_py_proto",
    deps = [":domain_knowledge_proto"],
)

proto_library(
    name = "eesi_proto",
    srcs = ["eesi.proto"],
    deps = [
        ":bitcode_proto",
        ":domain_knowledge_proto",
        ":operations_proto",
    ],
)

proto_library(
    name = "gpt_proto",
    srcs = ["gpt.proto"],
    deps = [
        ":eesi_proto",
    ],
)

py_proto_library(
    name = "eesi_py_proto",
    deps = [":eesi_proto"],
)

py_grpc_library(
    name = "eesi_py_grpc",
    srcs = [":eesi_proto"],
    deps = [":eesi_py_proto"],
)

cc_proto_library(
    name = "eesi_cc_proto",
    deps = ["eesi_proto"],
)

cc_grpc_library(
    name = "eesi_cc_grpc",
    srcs = [":eesi_proto"],
    grpc_only = True,
    deps = [":eesi_cc_proto"],
)

cc_proto_library(
    name = "gpt_cc_proto",
    deps = ["gpt_proto"],
)

cc_grpc_library(
    name = "gpt_cc_grpc",
    srcs = [":gpt_proto"],
    grpc_only=True,
    deps = [":gpt_cc_proto"],
)

py_proto_library(
    name = "gpt_py_proto",
    deps = ["gpt_proto"],
)

py_grpc_library(
    name = "gpt_py_grpc",
    srcs = [":gpt_proto"],
    deps = [":gpt_py_proto"],
)

proto_library(
    name = "status_proto",
    srcs = ["status.proto"],
    deps = [
        "@com_google_protobuf//:any_proto",
    ],
)

py_proto_library(
    name = "status_py_proto",
    deps = [":status_proto"],
)

py_grpc_library(
    name = "status_py_grpc",
    srcs = [":status_proto"],
    deps = [":status_py_proto"],
)

proto_library(
    name = "operations_proto",
    srcs = ["operations.proto"],
    deps = [
        ":status_proto",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:empty_proto",
    ],
)

py_proto_library(
    name = "operations_py_proto",
    deps = [":operations_proto"],
)

py_grpc_library(
    name = "operations_py_grpc",
    srcs = [":operations_proto"],
    deps = [":operations_py_proto"],
)

cc_proto_library(
    name = "operations_cc_proto",
    deps = ["operations_proto"],
)

cc_grpc_library(
    name = "operations_cc_grpc",
    srcs = [":operations_proto"],
    grpc_only = True,
    deps = [":operations_cc_proto"],
)
