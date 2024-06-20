py_library(
    name = "args",
    srcs = ["args.py"],
    visibility = [
        "//cli:__pkg__",
    ],
    deps = [
        ":commands",
        ":domain_knowledge_handler",
        ":synonym_configuration_handler",
        "//cli/common:service_configuration_handler",
        "//cli/db",
    ],
)

py_library(
    name = "commands",
    srcs = ["commands.py"],
    visibility = [
        "//cli/test/eesi:__pkg__",
    ],
    deps = [
        ":db",
        ":lattice_helper",
        ":rpc",
        "//cli/bitcode:db",
        "//cli/bitcode:rpc",
        "//cli/common:log",
        "//cli/common:uri",
        "//cli/db",
        "//proto:eesi_py_proto",
    ],
)

py_library(
    name = "lattice_helper",
    srcs = ["lattice_helper.py"],
    visibility = [
        "//cli/test/eesi:__pkg__",
    ],
    deps = [
        "//proto:eesi_py_proto",
    ],
)

py_library(
    name = "synonym_configuration_handler",
    srcs = ["synonym_configuration_handler.py"],
    visibility = [
        "//cli/test/eesi:__pkg__",
    ],
    deps = ["//proto:eesi_py_proto"],
)

py_library(
    name = "domain_knowledge_handler",
    srcs = ["domain_knowledge_handler.py"],
    visibility = [
        "//cli/getgraph:__pkg__",
        "//cli/test/eesi:__pkg__",
        "//cli/test/getgraph:__pkg__",
    ],
    deps = [
        "//proto:domain_knowledge_py_proto",
        "//proto:eesi_py_proto",
    ],
)

py_library(
    name = "db",
    srcs = ["db.py"],
    deps = [
        "//cli/db",
        "//proto:eesi_py_proto",
    ],
)

py_library(
    name = "rpc",
    srcs = ["rpc.py"],
    deps = [
        ":db",
        "//cli/operations:wait",
    ],
)
