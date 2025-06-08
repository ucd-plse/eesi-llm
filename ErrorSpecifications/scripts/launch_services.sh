SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BAZEL=bazel-4.1.0
DB_DIR=~/data/db
OPENAI_API_KEY=ADD-YOUR-KEY

tmux new -d -s bitcode "export GLOG_logtostderr=0 && export GLOG_log_dir=${SCRIPT_DIR}/../logs && cd ${SCRIPT_DIR}/.. && ${BAZEL} run //bitcode:main --cxxopt='-std=c++14'" 
tmux new -d -s eesi "export GLOG_logtostderr=0 && export GLOG_log_dir=${SCRIPT_DIR}/../logs && cd ${SCRIPT_DIR}/.. && ${BAZEL} run //eesi:main --cxxopt='-std=c++14'" 
tmux new -d -s gpt "export GLOG_logtostderr=0 && export GLOG_log_dir=${SCRIPT_DIR}/../logs && cd ${SCRIPT_DIR}/.. && OPENAI_API_KEY=${OPENAI_API_KEY} ${BAZEL} run //gpt:service --cxxopt='-std=c++14'" 
if [ ! -d $DB_DIR ]; then
    mkdir -p $DB_DIR 
fi
tmux new -d -s mongo "mongod --dbpath ${DB_DIR}"
