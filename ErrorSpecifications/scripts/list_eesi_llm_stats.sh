SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BASE_BC_DIR="file://$(cd ${SCRIPT_DIR}/../testdata/benchmarks/bitcode; pwd)/"
GT_DIR=$( cd ${SCRIPT_DIR}/../testdata/benchmarks/ground_truth; pwd )
LLM_NAME=gpt-4_1-mini-2025-04-14

mkdir -p ${SCRIPT_DIR}/../testdata/benchmarks/stats/${LLM_NAME}_eesi_llm
# Iterating through the thresholds like this is annoying and redundant, but
# I do not want to change this ATM.
for t in 1 100; do
    for prj in "zlib" "pidgin" "littlefs" "netdata" "mbedtls" "httpd"; do
      bazel run //cli:main -- --db-name ${LLM_NAME}_eesi_llm eesi ListStatisticsDatabase \
          --bitcode-uri ${BASE_BC_DIR}${prj}-reg2mem.bc \
          --ground-truth-path ${GT_DIR}/${prj}-gt.txt \
          --confidence-threshold ${t} \
          > ${SCRIPT_DIR}/../testdata/benchmarks/stats/${LLM_NAME}_eesi_llm/${prj}-${t}-stats.txt
    done
done
