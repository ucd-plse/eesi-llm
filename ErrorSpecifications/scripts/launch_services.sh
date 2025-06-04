SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BAZEL=bazel-4.1.0
DB_DIR=~/data/db

tmux new -d -s bitcode "cd ${SCRIPT_DIR}/.. && ${BAZEL} run //bitcode:main --cxxopt='-std=c++14'"
tmux new -d -s eesi "cd ${SCRIPT_DIR}/.. && ${BAZEL} run //eesi:main --cxxopt='-std=c++14'"
tmux new -d -s gpt "cd ${SCRIPT_DIR}/.. && ${BAZEL} run //gpt:service --cxxopt='-std=c++14'"
if [ ! -d $DB_DIR ]; then
    mkdir -p $DB_DIR 
fi
tmux new -d -s mongo "mongod --dbpath ~/data/db"
