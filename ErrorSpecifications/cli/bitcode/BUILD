py_library(
    name = "args",
    srcs = ["args.py"],
    visibility = [
        "//cli:__pkg__",
    ],
    deps = [
        ":commands",
        "//cli/common:service_configuration_handler",
        "//cli/db",
    ],
)

py_library(
    name = "commands",
    srcs = ["commands.py"],
    visibility = [
        "//cli/test/bitcode:__pkg__",
        "//cli/test/checker:__pkg__",
        "//cli/test/eesi:__pkg__",
        "//cli/test/getgraph:__pkg__",
    ],
    deps = [
        ":db",
        ":rpc",
        "//cli/common:uri",
        "//cli/db",
    ],
)

py_library(
    name = "db",
    srcs = ["db.py"],
    visibility = [
        "//cli/checker:__pkg__",
        "//cli/eesi:__pkg__",
        "//cli/getgraph:__pkg__",
        "//cli/test/bitcode:__pkg__",
    ],
    deps = [
        "//cli/common:uri",
        "//cli/db",
        "//proto:bitcode_py_proto",
    ],
)

py_library(
    name = "rpc",
    srcs = ["rpc.py"],
    visibility = [
        "//cli/checker:__pkg__",
        "//cli/eesi:__pkg__",
        "//cli/getgraph:__pkg__",
        "//cli/test/bitcode:__pkg__",
    ],
    deps = [
        ":db",
        "//cli/operations:wait",
        "//proto:bitcode_py_grpc",
    ],
)
