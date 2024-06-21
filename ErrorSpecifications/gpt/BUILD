py_binary(
    name = "service",
    srcs = ["service.py"],
    visibility = ["//gpt/test:__pkg__"],
    deps = [
        "//proto:bitcode_py_proto",
        "//proto:eesi_py_proto",
        "//proto:domain_knowledge_py_proto",
        "//proto:gpt_py_grpc",
        "//proto:operations_py_proto",
        "//proto:status_py_proto",
    ],
)
