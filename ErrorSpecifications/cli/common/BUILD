py_library(
    name = "log",
    srcs = ["log.py"],
    visibility = ["//visibility:public"],
)

py_library(
    name = "uri",
    srcs = ["uri.py"],
    visibility = ["//visibility:public"],
    deps = [
        "//proto:operations_py_grpc",
        "//proto:status_py_proto",
    ],
)

py_library(
    name = "service_configuration_handler",
    srcs = ["service_configuration_handler.py"],
    visibility = ["//visibility:public"],
    deps = [
        "//proto:bitcode_py_grpc",
        "//proto:domain_knowledge_py_proto",
        "//proto:eesi_py_grpc",
        "//proto:operations_py_grpc",
        "//proto:status_py_proto",
    ],
)
