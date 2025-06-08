#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
echo $SCRIPT_DIR
BASE_BC_DIR="file://$(cd ${SCRIPT_DIR}/../testdata/benchmarks/bitcode; pwd)/"
BASE_DK_DIR="$(cd ${SCRIPT_DIR}/../testdata/benchmarks/domain_knowledge; pwd)/"
BASE_SRC_DIR="$(cd ${SCRIPT_DIR}/../testdata/benchmarks/src_code; pwd)/"
BASE_EMBED_DIR="file://$(cd ${SCRIPT_DIR}/../testdata/benchmarks/embeddings; pwd)/"
MBEDTLS="${BASE_BC_DIR}mbedtls-reg2mem.bc" 
LFS="${BASE_BC_DIR}littlefs-reg2mem.bc" 
NETDATA="${BASE_BC_DIR}netdata-reg2mem.bc" 
PIDGIN="${BASE_BC_DIR}pidgin-reg2mem.bc" 
ZLIB="${BASE_BC_DIR}zlib-reg2mem.bc"
HTTPD="${BASE_BC_DIR}httpd-reg2mem.bc"
BAZEL=bazel-4.1.0
BAZEL_COPTS=(-Wno-error=array-parameter -Wno-error=stringop-overflow)
BAZEL_CXXOPTS=(-std=c++14)
LLM_NAME=gpt-4.1-mini-2025-04-14
DBNAME=${LLM_NAME}_eesi_llm
DBNAME=$(echo "$DBNAME" | tr . _)

declare -A error_codes=([$(basename $MBEDTLS .bc)]="${BASE_DK_DIR}error-code-mbedtls.txt" 
                        [$(basename $LFS .bc)]="${BASE_DK_DIR}error-code-littlefs.txt" 
                        [$(basename $NETDATA .bc)]="" 
                        [$(basename $PIDGIN .bc)]=""
                        [$(basename $HTTPD .bc)]="${BASE_DK_DIR}error-code-httpd.txt" 
                        [$(basename $ZLIB .bc)]="${BASE_DK_DIR}error-code-zlib.txt")
declare -A error_onlys=([$(basename $MBEDTLS .bc)]="${BASE_DK_DIR}error-only-mbedtls.txt" 
                        [$(basename $LFS .bc)]="" 
                        [$(basename $NETDATA .bc)]="" 
                        [$(basename $HTTPD .bc)]="" 
                        [$(basename $PIDGIN .bc)]="" 
                        [$(basename $ZLIB .bc)]="")
declare -A initial_specifications=([$(basename $MBEDTLS .bc)]="${BASE_DK_DIR}input-specs-mbedtls.txt" 
                                   [$(basename $LFS .bc)]="${BASE_DK_DIR}input-specs-littlefs.txt" 
                                   [$(basename $NETDATA .bc)]="${BASE_DK_DIR}input-specs-netdata.txt" 
                                   [$(basename $PIDGIN .bc)]="${BASE_DK_DIR}input-specs-pidgin.txt" 
                                   [$(basename $HTTPD .bc)]="${BASE_DK_DIR}input-specs-httpd.txt" 
                                   [$(basename $ZLIB .bc)]="${BASE_DK_DIR}input-specs-zlib.txt")
declare -A success_codes=([$(basename $MBEDTLS .bc)]="${BASE_DK_DIR}success-code-mbedtls.txt" 
                          [$(basename $LFS .bc)]="${BASE_DK_DIR}success-code-littlefs.txt" 
                          [$(basename $NETDATA .bc)]="" 
                          [$(basename $PIDGIN .bc)]="" 
                          [$(basename $HTTPD .bc)]="${BASE_DK_DIR}success-code-httpd.txt" 
                          [$(basename $ZLIB .bc)]="${BASE_DK_DIR}success-code-zlib.txt" )
declare -A smart_success_code_zeros=([$(basename $MBEDTLS .bc)]="--smart-success-code-zero" 
                                     [$(basename $LFS .bc)]="--smart-success-code-zero" 
                                     [$(basename $NETDATA .bc)]="" 
                                     [$(basename $PIDGIN .bc)]="" 
                                     [$(basename $HTTPD .bc)]="--smart-success-code-zero" 
                                     [$(basename $ZLIB .bc)]="--smart-success-code-zero" )
# These paths assume the benchmarks have been extracted to ./testdata/benchmarks/src_code/
declare -A ctag_files=([$(basename $MBEDTLS .bc)]="${BASE_SRC_DIR}mbedtls/tags" 
                       [$(basename $LFS .bc)]="${BASE_SRC_DIR}littlefs-1.7.0/tags" 
                       [$(basename $NETDATA .bc)]="${BASE_SRC_DIR}netdata-1.11.0_rolling/tags" 
                       [$(basename $HTTPD .bc)]="${BASE_SRC_DIR}httpd-2.4.46/tags" 
                       [$(basename $PIDGIN .bc)]="${BASE_SRC_DIR}pidgin-otrng/tags" 
                       [$(basename $ZLIB .bc)]="${BASE_SRC_DIR}zlib-1.2.11/tags")

usage() { echo "Usage: $0 [-d <DB_BASE_NAME>] [-e <EVID_COUNT>]" 1>&2;
          echo "          [-z zlib] [-p pidgin] [-n netdata] [-m mbedtls]" 1>&2;
          echo "          [-l littlefs] [-a httpd] [-o overwrite]" 1>&2; exit 1; }

declare -a bc_uris=( "$LFS" "$PIDGIN" "$MBEDTLS" "$ZLIB" "$NETDATA" "$HTTPD" )
OVERWRITE=""
while getopts "d:osl:zpanml" o; do
    case "${o}" in
        d)
            DBNAME=${OPTARG}
            ;;
        z)
            bc_uris=( "$ZLIB" )
            ;;
        p)
            bc_uris=( "$PIDGIN" )
            ;;
        n)
            bc_uris=( "$NETDATA" )
            ;;
        m)
            bc_uris=( "$MBEDTLS" )
            ;;
        l)
            bc_uris=( "$LFS" )
            ;;
        a)
            bc_uris=( "$HTTPD" )
            ;;
        o)
            OVERWRITE="--overwrite"
            ;;
        *)
            usage
            ;;
        esac
done

for bc_uri in ${bc_uris[@]}; do
    bc_basename="$(basename $bc_uri .bc)"
    error_code="--error-codes ${error_codes[$bc_basename]}"
    if [ "${error_code}" == "--error-codes " ]; then
      error_code=""
    fi
    error_only="--error-only ${error_onlys[$bc_basename]}"
    if [ "${error_only}" == "--error-only " ]; then
      error_only=""
    fi
    init_specs="--initial-specifications ${initial_specifications[$bc_basename]}"
    if [ "${init_specs}" = "--initial-specifications " ]; then
      init_specs=""
    fi
    success_code="--success-codes ${success_codes[$bc_basename]}"
    if [ "${success_code}" = "--success-codes " ]; then
      success_code=""
    fi
    smart_success_code_zero="${smart_success_code_zeros[$bc_basename]}"
    ctags="--ctags ${ctag_files[$bc_basename]}"
    
    ${BAZEL} run //cli:main  "${BAZEL_CXXOPTS[@]/#/--cxxopt=}" -- --db-name ${DBNAME} \
      bitcode RegisterBitcode --uri $bc_uri 
    ${BAZEL} run //cli:main  "${BAZEL_CXXOPTS[@]/#/--cxxopt=}" -- --db-name ${DBNAME} \
        bitcode GetCalledFunctionsUri --uri $bc_uri
    ${BAZEL} run //cli:main  "${BAZEL_CXXOPTS[@]/#/--cxxopt=}" -- --db-name ${DBNAME} \
        bitcode GetDefinedFunctionsUri --uri $bc_uri
    ${BAZEL} run //cli:main  "${BAZEL_CXXOPTS[@]/#/--cxxopt=}" -- --db-name ${DBNAME} \
        eesi GetSpecificationsUri --bitcode-uri $bc_uri $ctags $init_specs \
        $error_code $error_only $success_code $smart_success_code_zero \
        --llm-name ${LLM_NAME} ${OVERWRITE}
done
