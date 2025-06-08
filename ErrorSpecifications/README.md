# Interleaving Static Analysis and LLM Prompting - Tool Implementation

### Requirements

You can either try to set up all of the requirements and dependencies on your
own system to reproduce the experiments or you can use the provided [docker](https://docs.docker.com/get-started/get-docker/)
image that provides an environment with the necessary dependencies to run the
interleaved analysis and run the experiments on the benchmarks that were
demonstrated in the papers. 

#### OpenAI API Token (Required for All)

You must have an OpenAI API token to use the interleaved analysis. You can
get one by visiting their [website](https://www.platform.openai.com/api-keys).
You then must set the environment variable `OPENAI_API_KEY` to the token that
you have generated. If you have not set this up properly, then the GPT service
introduced later will not start properly.

#### Setting up the Docker Image

First, ensure that you have [docker installed](https://www.docker.com/get-started/).
Afterwards, you can build the image using the command (~5-10 minutes):
```bash
cd ./docker && docker build --tag "eesillm" .
```

This will then build the image that provides that required dependencies. After
the image has finished building, you can start the container and enter the shell
with the command:
```bash
docker run -v <PATH-TO-REPO-DIR>:/home/evaluation-container/eesi-llm -it eesillm /bin/bash
```

You then enter into the directory:
 ```bash
cd /home/evaluation-container/eesi-llm`
```

And you will now have the required environment to run the interleaved analysis.
Please note that the directory `/home/evaluation-container/eesi-llm` is where
the shared volume between the host and container is located, so any changes
made to files in one will affect the other. The remainder of the commands related
to running the script will apply to both running the commands in this container
and any local machine that is set up.

#### Local Setup (Ubuntu 20.04)
To install the initial set of dependencies:
```bash
$ ./scripts/install_deps.sh
```

You can also install `bazel 4.1.0` and `MongoDB`:

- [bazel 4.1.0](https://docs.bazel.build/versions/4.1.0/install-ubuntu.html)
- [MongoDB](https://www.mongodb.com/docs/manual/tutorial/install-mongodb-on-ubuntu/)

#### bazel_python (aprox. running time: 15-20 minutes)

EESI depends on the `bazel_python` package. To install and setup
`bazel_python`, please visit the [repository](https://github.com/95616ARG/bazel_python/)
and follow the `README`. Note: We use python `3.7.4` as demonstrated in the
`bazel_python` `README`. If you do not use `bazel-4.1.0`, installing `bazel_python` will
likely not work. We have also included a `bazel_python.tar.gz` file with
the compressed version.

### Initial Setup

#### Launching Services

##### Interleaved Analyis Services

The analysis pipeline is implemented as gRPC services, in order to run the
analysis, you must launch each service. You can do so automatically by
running the script:
```bash
$ ./script/launch_services.sh
``` 

Note: you must have tmux installed for the previous script to work. The previous
script will launch services in tmux sessions: `bitcode`, `eesi`, `gpt` and `mongo`. 

You can also launch the services manually by:
```bash
$ bazel run //bitcode:main --cxxopt='-std=c++14' --copt="-Wno-error=array-parameter" --copt="-Wno-error=stringop-overflow"
$ bazel run //eesi:main --cxxopt='-std=c++14' --copt="-Wno-error=array-parameter" --copt="-Wno-error=stringop-overflow"
$ bazel run //gpt:service --cxxopt='-std=c++14' --copt="-Wno-error=array-parameter" --copt="-Wno-error=stringop-overflow"
$ mongod --port 27017
```

### Running the Tool

You can either re-run the interleaved analysis on the same benchmarks presented
in the paper by following the directions in [Reproducing Analysis from Paper](####Reproducing-Analysis-from-Paper)
or you can run your own analysis by following the directions in [Running Analysis on Your Own](####Running-Analysis-on-Your-Own).

#### Reproducing Analysis from Paper 

To replicate the results in Interleaving Static Analysis and LLM Prompting,
refer to the follow section. Note: Due to the random nature of LLMs, you will
almost certainly not get the same results running the entire analysis from
scratch. We have included the results that we obtained during our runs, 

##### Benchmark Bitcode

In order to replicate the results for EESI, please extract the bitcode files
by doing the following:
```bash
$ cd ./testdata/benchmarks/bitcode && tar -xf benchmarks.tar.gz && cd -
```

You should now have the following files in your `./testdata/benchmarks/bitcode`
folder:
```
benchmarks.tar.gz   littlefs-reg2mem.bc  httpd-reg2mem.bc
mbedtls-reg2mem.bc  netdata-reg2mem.bc   pidgin-reg2mem.ll
zlib-reg2mem.bc
```

##### Benchmark Source Code

The source code for all of the analyzed benchmarks presented in the papers
are too large for this repo. You can download a compressed tar file from
the [following link](https://drive.google.com/file/d/1tB8mEghS_zkvT3DYvgbiLOguvGQvCdQi/view?usp=share_link).
Move the downloaded `.tar` file to `./testdata/benchmarks` and then extract
the source code using:
```bash
cd ./testdata/benchmarks/ && tar -xf ./benchmarks-src-code.tar.gz && cd -
```

Note: The helper scripts for reproducing paper results expect the benchmark
source code to be in the location described above.

##### Reproducing Evaluation Results (approx. running time: 30 minutes, ~20 GB memory) 

As the LLM ran in the experiments is [GPT-4](https://openai.com/index/gpt-4/), the results
are inherently non-deterministic from the LLM. It is extremely likely that your results
will not match the paper results exactly. Also, model changes could potentially impact
the results in the future, either negatively or positively. As such, our framework is
designed to be LLM-agnostic, and you can set up your own LLM of choice (from OpenAI).
The included helper scripts will run with GPT-4.1-mini by default, as it is a mix
of being cost-effective with reasonable performance. If you simply wish to just
view the final results of our interleaved analysis, we have included the learned
specifications and a helper script to view the final set of learned error specifications.

**Injecting Specifications**: To view the final results of our interleaved analysis
(the numbers from Table 4 in both SOAP and STTT papers), you can use the
`./scripts/inject_specifications.sh` helper script to inject the specifications
from the files included in `./testdata/benchmarks/artifact_results`. You can
then view the raw number of learned error specifications using the command:
```bash
bazel run //cli:main -- --db-name eesi_llm_injected eesi ListSpecificationsTable
```
Which should look like:
```bash
Bitcode ID (last 8 characters)           File name:            <0          >0          ==0         <=0         >=0         !=0         top         emptyset    total        increase %
3b4bddf4                                 httpd-reg2mem.bc      46          98           42           2           6           93          0           534          821         0.00      
614e5c1f                                 littlefs-reg2mem.bc   50          0            7            0           0           0           0           10           67          0.00      
ef786082                                 mbedtls-reg2mem.bc    818         15           64           4           0           1           0           272          1174        0.00      
91d8f71a                                 netdata-reg2mem.bc    161         72           222          2           4           1           0           234          696         0.00      
e7471634                                 pidgin-reg2mem.bc     16          4            95           0           4           0           0           53           172         0.00      
cd77a397                                 zlib-reg2mem.bc       76          1            14           0           0           0           0           29           120         0.00      
```
Please note that these injected results are for just simply viewing the raw
numbers for comparison against the numbers presented in the paper.

**Running Analysis**: To run EESI and the interleaved LLM analysis, you can use
the script `./scripts/run_benchmarks.sh`. Due to the size of the benchmarks
being analyzed, the memory usage for EESI can be quite high relative to
some systems, approximately ~20 GB in the case of running on all benchmarks.
If you wish to just run on a select benchmark, you can refer to the usage:
```bash
$ ./scripts/run_benchmarks.sh [-z zlib] [-p pidgin] [-n netdata] [-m mbedtls]
                              [-l littlefs] [-a httpd] [-o overwrite] 
```

If you already have results stored in the database, you can supply the `-o`
flag which will overwrite the currently stored results. Note: If you just want
to get a quick result, you can just run the analysis on the LittleFS benchmark
using `-l`, as it is the smallest.


##### Specification Counts (approx. running time: <1 minute)

To view the specifications counts, refer to the `ListSpecificationsTable` commands
and use the appropriate `--db-name` related to your experimental results of
interest. For example, if you store the baseline static analysis results from
EESI (`Table 3` in STTT and SOAP papers) where the results are stored in `gpt-4_1-mini-2024-04-14_eesi_llm`:

```bash
bazel run //cli:main -- --db-name gpt-4_1-mini-2024-04-14_eesi_llm eesi ListSpecificationsTable --confidence-threshold 100
```

The output should look like:
```
Bitcode ID (last 8 characters)           File name:                                                                  <0          >0          ==0         <=0         >=0         !=0         top         emptyset    total        increase %
614e5c1f                                 littlefs-reg2mem.bc                                                         40          0            7            0           0           0           0           10           57          0.00      
ef786082                                 mbedtls-reg2mem.bc                                                          723         10           48           3           0           1           0           246          1031        0.00      
91d8f71a                                 netdata-reg2mem.bc                                                          17          35           108          0           1           1           0           116          278         0.00      
4bd676a1                                 pidgin-reg2mem.bc                                                           11          4            24           0           0           0           0           29           68          0.00      
cd77a397                                 zlib-reg2mem.bc                                                             68          1            14           0           0           0           0           29           112         0.00  
cd77a397                                 httpd-reg2mem.bc                                                            16          42           16           0           1           27          0           183          285         0.00  
```

Note: The above results are non-deterministic, as the above command will
cut-off any error specifications learned via LLM-assisted analysis. The remainder
of the `List` commands will result in non-deterministic numbers.

If you run the interleaved analysis, you can view your results by supplying the correct
`--db-name` (e.g., `gpt-4_1-mini-2024-04-14_eesi_llm`) and changing the `--confidence-threshold` value
to `1`. Remember, if you ran the analysis locally, it is extremely likely your results
will not match the paper. The results for our individual experimental run
were presented in Table 4 of both the STTT and SOAP papers.

##### Precision, Recall, and F1 (approx. running time: 1-5 minutes)

After having learned the error specifications using the interleaved analysis, you
can dump a simple statistics summary using the helper script `./scripts/list_eesi_llm_stats.sh`.
These will dump `*.txt` files into the `testdata/benchmarks/stats` directory
for the experimental results in the `gpt-4_1-mini-2025-04-14_eesi_llm` database.
Each `*.txt` file will correspond to the benchmark results stored in the database
for the baseline analysis (without the LLM-assistance) and the interleaved
analysis. These numbers represent the raw metrics that are presented in the
remaining figures regarding precision, recall, and F1.

#### Running Analysis on Your Own

##### Registering Bitcode
To run the tool on some arbitrary program, you must first register the bitcode
that will be analyzed by EESI. If you don't have the bitcode set up, read
the [generating bitcode](#####Generating-Bitcode) section:
```
bazel run //cli:main -- --db-name <DB-NAME> bitcode RegisterBitcode \
    --bitcode-uri <BITCODE-URI> 
```

You will also need to calculate called and defined functions, these will either
be calculated when using the `GetSpecifications` commands or separately by
providing commands:
```
bazel run //cli:main -- --db-name <DB-NAME> bitcode GetCalledFunctinosUri
    --bitcode-uri <BITCODE-URI>
```
and
```
bazel run //cli:main -- --db-name <DB-NAME> bitcode GetDefinedFunctinosUri
    --bitcode-uri <BITCODE-URI>
```

You may replace the `Uri` portion of the command with `All` (e.g., `GetDefinedFunctionsAll`)
if you wish to calculate these functions for all registered bitcode files.

#### Running the Analysis

If you wish to use the LLM for error specification inference, please make sure
that you have generated a `ctags` at the root of the project directory that
will be analyzed:
```
ctags --field=+ne -R <PROJECT-ROOT-DIRECTORY>
```

Then, to perform the error specification inference, you run the command
using the CLI:
```
bazel run //cli:main -- --db-name <DB-NAME> eesi GetSpecificationsUri
    --bitcode-uri <BITCODE-URI> --tags <CTAGS-FILE> [--initial-specifications <INIT-SPECS>]
    [--error-codes <ERROR-CODES>] [--success-codes <SUCCESS-CODES>] [--error-only <ERROR-ONLY>]
```

The tool should then perform the analysis, inferring error specifications. You
may view the raw list of inferred error specifications using:
```
bazel run //cli:main -- --db-name <DB-NAME> eesi ListSpecifications --bitcode-uri <BITCODE-URI>
```
Or, you may also view the specifications table count (for all registered bitcode):
```
bazel run //cli:main -- --db-name <DB-NAME> eesi ListSpecificationsTable
```
##### Generating Bitcode

Bitcode files can be generated using [gllvm](https://github.com/SRI-CSL/gllvm)
and following their instructions.

You can also use `clang` with LLVM 7.1 support. For example generating a bitcode
file for `hello.c`:

```bash
clang -S -O0 -g -emit-llvm hello.c
```

The command will result in a human-readable `hello.ll` file.

You can also use the command:
```bash
clang -c -O0 -g -emit-llvm hello.c
```
