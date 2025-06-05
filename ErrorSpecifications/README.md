# Interleaving Static Analysis and LLM Prompting 

### Requirements
To install dependencies:
```bash
$ ./scripts/install_deps.sh
```

You can install `bazel 4.1.0` and `MongoDB 3.6`:

- [bazel 4.1.0](https://docs.bazel.build/versions/4.1.0/install-ubuntu.html)
- [MongoDB 3.6](https://www.mongodb.com/docs/manual/tutorial/install-mongodb-on-ubuntu/)

To install dependencies to run with (2) docker:
```bash
$ ./scripts/setup_docker.sh
```

`Docker` is located at:
- [Docker](https://docs.docker.com/engine/install/)


#### bazel_python (aprox. running time: 15-20 minutes)

EESI depends on the `bazel_python` package. To install and setup
`bazel_python`, please visit the [repository](https://github.com/95616ARG/bazel_python/)
and follow the `README`. Note: We use python `3.7.4` as demonstrated in the
`bazel_python` `README`. If you do not use `bazel-4.1.0`, installing `bazel_python` will
likely not work.

### Initial Setup

#### Launching Services

##### EESIER Services

The EESI pipeline is implemented as gRPC services, in order to run the
analysis, you must launch each service. You can do so automatically by
running the script:
```bash
$ ./script/launch_services.sh
``` 

Note: you must have tmux installed for the previous script to work. The previous
script will launch services in tmux sessions: `bitcode`, `eesi`, and `gpt`. 

You can also launch the services manually by:
```bash
$ bazel run //bitcode:main
$ bazel run //eesi:main
$ bazel run //gpt:service
```

##### Mongo

Results for EESI are stored in Mongo. Before running EESI, make sure that
you have installed Mongo and have the mongo service active and listening to
the default port of `27017`. To start the mongo service:
```bash
$ mongod --port 27017
```

### Running the Tool

#### Registering Bitcode
To run the tool on some arbitrary program, you must first register the bitcode
that will be analyzed by EESI:
```
bazel run //clli:main -- --db-name <DB-NAME> bitcode RegisterBitcode \
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

### Replicating Evaluation Results

To replicate the results in Interleaving Static Analysis and LLM Prompting,
refer to the follow section. Note: Due to the random nature of LLMs, you will
almost certainly not get the same results running the entire analysis from
scratch. We have included the results that we obtained during our runs, 

#### Benchmark Bitcode

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


#### Replroducing Evaluation Results (approx. running time: 30 minutes, ~20 GB memory) 

As the LLM ran in the experiments is [GPT-4](https://openai.com/index/gpt-4/), the results
are inherently non-deterministic from the LLM. It is extremely likely that your results
will not match the paper results exactly. Also, model changes could potentially impact
the results in the future, either negatively or positively. As such, our framework is
designed to be LLM-agnostic, and you can set up your own LLM of choice.

To run EESI and the interleaved LLM analysis, you can use
the script `./scripts/run_benchmarks.sh`. Due to the size of the benchmarks
being analyzed, the memory usage for EESI can be quite high relative to
some systems, approximately ~20 GB in the case of running on all benchmarks.
If you wish to just run on a select benchmark, you can refer to the usage:
```bash
$ ./scripts/run_benchmarks.sh [-z zlib] [-p pidgin] [-n netdata] [-m mbedtls]
                              [-l littlefs] [-s openssl] [-o overwrite] 
```

If you already have results stored in the database, you can supply the `-o`
flag which will overwrite the currently stored results.


##### Specification Counts (approx. running time: <1 minute)

To view the specifications counts, refer to the `ListSpecificationsTable` commands
and use the appropriate `--db-name` related to your experimental results of
interest. For example, if you store the baseline static analysis results from
EESI (`Table 3`) where the results are stored in `eesi_results`:

```bash
bazel run //cli:main -- --db-name eesi_results eesi ListSpecificationsTable --confidence-threshold 100
```

The output should look like:
```
Bitcode ID (last 8 characters)           File name:                                                                  <0          >0          ==0         <=0         >=0         !=0         top         emptyset    total        increase %
614e5c1f                                 littlefs-reg2mem.bc                                                         40          0            7            0           0           0           0           10           57          0.00      
ef786082                                 mbedtls-reg2mem.bc                                                          723         10           48           3           0           1           0           246          1031        0.00      
91d8f71a                                 netdata-reg2mem.bc                                                          17          35           108          0           1           1           0           116          278         0.00      
4bd676a1                                 pidgin-reg2mem.bc                                                           11          4            24           0           0           0           0           29           68          0.00      
cd77a397                                 zlib-reg2mem.bc                                                             68          1            14           0           0           0           0           29           112         0.00  
```

If you run the interleaved analysis, you can view your results by supplying the correct
`--db-name` (e.g., `eesi_llm_results`) and changing the `--confidence-threshold` value
to `1`. Remember, if you ran the analysis locally, it is extremely likely your results
will not match the paper.

##### Precision, Recall, and F1 (approx. running time: 1-5 minutes)

After having generated the specifications for the EESI baseline, you can calculate statistics
for benchmarks by running the script:
```bash
$ ./scripts/list_eesi_stats.sh
```

Running this script should take approximately one minute. This script will populate the
`./testdata/benchmarks/stats/` folder with statistics for the
benchmarks that specifications were found for by EESI. You can then list and
view the statistics that were presented in the paper.
 or `100` in the name of the file. The number indicates the confidence
threshold that was applied, where the `1` confidence indicates the minimum
confidence threshold and `100` indicates the maximum confidence threshold:

## Generating New Bitcode

Bitcode files can be generated using [gllvm](https://github.com/SRI-CSL/gllvm)
and following their instructions.

You can also use `clang` with LLVM 7.0+ support. For example generating a bitcode
file for `hello.c`:

```bash
clang -S -O0 -g -emit-llvm hello.c
```

The command will result in a human-readable `hello.ll` file.

You can also use the command:
```bash
clang -c -O0 -g -emit-llvm hello.c
```
