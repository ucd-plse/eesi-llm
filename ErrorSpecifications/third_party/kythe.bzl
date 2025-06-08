load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")
load("//third_party:llvm_repo.bzl", "git_llvm_repository")

def kythe_deps():
    http_archive(
        name = "net_zlib",
        build_file = "@io_kythe//third_party:zlib.BUILD",
        sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
        strip_prefix = "zlib-1.2.11",
        urls = ["https://www.zlib.net/fossils/zlib-1.2.11.tar.gz"]
    )

    git_llvm_repository(
        name = "org_llvm",
    )

    http_archive(
        name = "io_kythe_llvmbzlgen",
        sha256 = "6d077cfe818d08ea9184d71f73581135b69c379692771afd88392fa1fee018ac",
        urls = ["https://github.com/kythe/llvmbzlgen/archive/435bad1d07f7a8d32979d66cd5547e1b32dca812.zip"],
        strip_prefix = "llvmbzlgen-435bad1d07f7a8d32979d66cd5547e1b32dca812",
    )

    native.bind(
        name = "zlib",
        actual = "@net_zlib//:zlib",
    )

    go_rules_dependencies()
    go_register_toolchains()
