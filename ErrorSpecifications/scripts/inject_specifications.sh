#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BASE_BC_DIR="file://$(cd ${SCRIPT_DIR}/../testdata/benchmarks/bitcode; pwd)/"
BASE_DK_DIR="$(cd ${SCRIPT_DIR}/../testdata/benchmarks/domain_knowledge; pwd)/"
BASE_RESULTS_DIR="$(cd ${SCRIPT_DIR}/../testdata/benchmarks/artifact_results; pwd)/"
MBEDTLS="mbedtls"
LFS="littlefs"
NETDATA="netdata"
HTTPD="httpd"
PIDGIN="pidgin"
ZLIB="zlib"
declare -a benchmarks=( "$LFS" "$PIDGIN" "$MBEDTLS" "$HTTPD" "$ZLIB" "$NETDATA" )
declare -a benchmarks=( "$HTTPD" )

for benchmark in ${benchmarks[@]}; do
    bazel run //cli:main -- --db-name eesi_llm_injected eesi InjectSpecifications --bitcode-uri ${BASE_BC_DIR}${benchmark}-reg2mem.bc --specifications ${BASE_RESULTS_DIR}${benchmark}-specifications.txt --initial-specifications ${BASE_DK_DIR}input-specs-${benchmark}.txt --overwrite
done
