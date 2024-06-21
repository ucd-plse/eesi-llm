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


#### Replicating EESIER Results (approx. running time: 30 minutes, ~20 GB memory) 

To run EESIER and reproduce the results from the Evaluation, you can use
the script `./scripts/run_benchmarks.sh`. Due to the size of the benchmarks
being analyzed, the memory usage for EESIER can be quite high relative to
some systems, approximately ~20 GB in the case of running on all benchmarks.
If you wish to just run on a select benchmark, you can refer to the usage:
```bash
$ ./scripts/run_benchmarks.sh [-z zlib] [-p pidgin] [-n netdata] [-m mbedtls]
                              [-l littlefs] [-s openssl] [-o overwrite] 
```

If you already have results stored in the database, you can supply the `-o`
flag which will overwrite the currently stored results.


##### Specification Counts (approx. running time: <1 minute)

To view the specifications counts tables from the paper, i.e. `Table VII` and
`Table IX`, you can use the `ListSpecificationsTable` command from the EESIER CLI.

To view ths specifications table for the EESIER analysis `Table 3`:
```bash
bazel run //cli:main -- --db-name eesier_evid3_meet eesi ListSpecificationsTable --confidence-threshold 100
```

The output should look like:
```
Bitcode ID (last 8 characters)           File name:                                                                  <0          >0          ==0         <=0         >=0         !=0         top         emptyset    total        increase %
614e5c1f                                 littlefs-reg2mem.bc                                                         40          0            7            0           0           0           0           10           57          0.00      
ef786082                                 mbedtls-reg2mem.bc                                                          723         10           48           3           0           1           0           246          1031        0.00      
91d8f71a                                 netdata-reg2mem.bc                                                          17          35           108          0           1           1           0           116          278         0.00      
4bd676a1                                 openssl-reg2mem.bc                                                          366         86           2940         351         35          13          0           2271         6062        0.00      
e7471634                                 pidgin-reg2mem.bc                                                           11          4            24           0           0           0           0           29           68          0.00      
cd77a397                                 zlib-reg2mem.bc                                                             68          1            14           0           0           0           0           29           112         0.00  
```

To view the specifications table for EESIER meet `Table 7`:
```bash
bazel run //cli:main -- --db-name eesier_evid3_meet eesi ListSpecificationsTable --confidence-threshold 1
```

The output should look like:
```
Bitcode ID (last 8 characters)           File name:                                                                  <0          >0          ==0         <=0         >=0         !=0         top         emptyset    total        increase %
614e5c1f                                 littlefs-reg2mem.bc                                                         43          0            8            0           0           0           0           12           63          10.53      
ef786082                                 mbedtls-reg2mem.bc                                                          763         14           51           3           0           1           0           310          1142        10.77      
91d8f71a                                 netdata-reg2mem.bc                                                          72          38           130          1           1           3           0           192          437         57.19      
4bd676a1                                 openssl-reg2mem.bc                                                          411         95           3175         358         40          16          0           2972         7067        16.58      
e7471634                                 pidgin-reg2mem.bc                                                           12          4            53           0           0           0           0           74           143         110.29      
cd77a397                                 zlib-reg2mem.bc                                                             69          1            14           0           0           0           0           33           117         4.46 
```

To view the specifications table for EESIER join `Table 8`:
```bash
bazel run //cli:main -- --db-name eesier_evid3_join eesi ListSpecificationsTable --confidence-threshold 1
```

The output should look like:
```
Bitcode ID (last 8 characters)           File name:                                                                  <0          >0          ==0         <=0         >=0         !=0         top         emptyset    total        increase %
614e5c1f                                 littlefs-reg2mem.bc                                                         44          0            8            0           0           0           0           11           63          10.53      
ef786082                                 mbedtls-reg2mem.bc                                                          805         15           60           4           0           3           0           254          1141        10.67      
91d8f71a                                 netdata-reg2mem.bc                                                          69          44           145          7           1           14          0           162          442         58.99      
4bd676a1                                 openssl-reg2mem.bc                                                          422         110          3522         435         51          21          0           2389         6950        14.65      
e7471634                                 pidgin-reg2mem.bc                                                           12          5            76           0           5           0           0           48           146         114.71      
cd77a397                                 zlib-reg2mem.bc                                                             69          1            14           0           0           0           0           33           117         4.46 
```

To view the specifications table EESIER max `Table 9`:
```bash
bazel run //cli:main -- --db-name eesier_evid3_max eesi ListSpecificationsTable --confidence-threshold 1
```

The output should look like:
```
Bitcode ID (last 8 characters)           File name:                                                                  <0          >0          ==0         <=0         >=0         !=0         top         emptyset    total        increase %
614e5c1f                                 littlefs-reg2mem.bc                                                         43          0            8            0           0           0           0           12           63          10.53      
ef786082                                 mbedtls-reg2mem.bc                                                          792         15           54           3           0           2           0           275          1141        10.67      
91d8f71a                                 netdata-reg2mem.bc                                                          75          38           148          1           1           5           0           179          447         60.79      
4bd676a1                                 openssl-reg2mem.bc                                                          438         108          3406         392         48          19          0           2585         6996        15.41      
e7471634                                 pidgin-reg2mem.bc                                                           12          6            66           0           0           0           0           60           144         111.76      
cd77a397                                 zlib-reg2mem.bc                                                             69          1            14           0           0           0           0           33           117         4.46
```

##### Precision and Recall for EESIER (approx. running time: 1-5 minutes)

After having generated the specifications for EESIER, you can calculate statistics
for benchmarks by running the script:
```bash
$ ./scripts/list_eesier_stats.sh
```

Running this script should take approximately one minute. This script will populate the
`./testdata/benchmarks/stats/` folder with statistics for the
benchmarks that specifications were found for by EESIER. You can then list and
view the statistics that were presented in `Table VI` and `Table VIII` by
running the script:
```bash

$ ./scripts/table6_8.sh
```

Running the script should take approximately less than one minute. The output
of this script should show two rows for each benchmark, with either
number `1` or `100` in the name of the file. The number indicates the confidence
threshold that was applied, where the `1` confidence indicates the minimum
confidence threshold and `100` indicates the maximum confidence threshold:

```
File                            Precision     Recall    F-score
littlefs-100-stats.txt             91.30%     75.41% 0.8259940145361265
littlefs-1-stats.txt               91.67%     98.36% 0.948957584471603
mbedtls-100-stats.txt              90.64%     83.40% 0.8687207802117003
mbedtls-1-stats.txt                85.52%     94.96% 0.8999286872779366
netdata-100-stats.txt              64.60%     30.29% 0.4124293785310734
netdata-1-stats.txt                48.23%     64.73% 0.5527509142624948
openssl-100-stats.txt              76.13%     77.38% 0.7675181892838903
openssl-1-stats.txt                72.65%     87.89% 0.7954776526321282
pidgin-100-stats.txt               76.32%     13.97% 0.23617659665666524
pidgin-1-stats.txt                 58.47%     43.38% 0.4981035115624618
zlib-100-stats.txt                 97.14%     82.68% 0.8932816214187415
zlib-1-stats.txt                   87.70%     96.06% 0.9169388543532468
```

This should be the final command to view the necessary tables for reproducing
results. The delta table, `Table X` was just calculated from the previous
statistics tables.


### Replicating EESI Results (for comparison against EESIER analysis) 

#### Initial Setup

Please move to the directory `./third_party/eesi/`, after run the script:
```bash
./src/scripts/setup.sh
```

This script just simply creates a directory for results and copies over bitcode
files for EESI to use for evaluation.


##### Specification Counts (approx. running time: 20-30 minute, ~15 GB memory)

Please move into the EESI directory located at `./third_party/eesi/` and follow the
[README](./third_party/eesi/README.md) for generating specifications on the
benchmarks. Also make sure that you follow the [installation guide](./third_party/eesi/INSTALL.md) for setting up EESI
and make sure that you run it within a docker container as described. Following
this guide will allow you to generate `Table IV` from the paper and will give
you specification results that are used to calculate precision, recall, F-score
at the [next step](######-Precision,-Recall,-and-F-Score-for-EESI) in this
README.

##### Precision and Recall for EESI (approx. running time: <1 minute)

After having generated the specifications for EESI from the previous steps, you
can calculate statistics for benchmarks by running the script:
```bash
$ ./scripts/list_eesi_stats.sh
```

Running this script should take approximately one minute. This script will populate the
`./third_party/eesi/results/artifact/stats/` folder with statistics for the
benchmarks that specifications were found for by EESI. You can then list and
view the statistics that were presented in `Table V` by running the script:
```bash

$ ./scripts/table5.sh
```

The resulting output:

```
File                            Precision     Recall    F-score
littlefs-stats.txt                 89.47%     62.30% 0.7345082433200684
mbedtls-stats.txt                  49.75%     63.67% 0.558567411700571
netdata-stats.txt                  40.85%     17.57% 0.24566435432230524
openssl-stats.txt                  81.93%     38.71% 0.5258056286399827
pidgin-stats.txt                   52.94%      6.25% 0.11180124223602485
zlib-stats.txt                     98.63%     57.48% 0.7263179713950114

```

## Additional Material
### Note: not related to artifact

When running EESIER on your own benchmarks, we suggest using `tcmalloc`!
Without `tcmalloc` it is likely that the services will consume excessive
amounts of memory. 

## Building and running tests
### Build and test commands
```bash
export LD_PRELOAD="/usr/lib/libtcmalloc.so.4"
bazel build --action_env=LD_PRELOAD //...
bazel test --action_env=LD_PRELOAD //...
```

#### Bitcode service tests
The bitcode service tests are in `bitcode/test`.
```bash
export LD_PRELOAD="/usr/lib/libtcmalloc.so.4"
bazel build --action_env=LD_PRELOAD //bitcode/...
bazel test --action_env=LD_PRELOAD //bitcode/...
```

#### EESI service tests
The EESI service tests are in `eesi/test/`.
```bash
export LD_PRELOAD="/usr/lib/libtcmalloc.so.4"
bazel build --action_env=LD_PRELOAD //eesi/...
bazel test --action_env=LD_PRELOAD //eesi/...
```

#### Checker service tests
The checker service tests are in `checker/test/`.
```bash
export LD_PRELOAD="/usr/lib/libtcmalloc.so.4"
bazel build --action_env=LD_PRELOAD //checker/...
bazel test --action_env=LD_PRELOAD //checker/...
```

## Running a service

Each service takes one parameter: the address and port to bind for listening. 
It will default to `localhost` with the standard gRPC port numbers starting
at `50051`. To listen on a different address, pass the `-listen` parameter.
If you want to run a publicly available service, then listen on 
`0.0.0.0`

The port that is chosen is unimportant.

Running the bitcode service:

```bash
# Starting the bitcode service listening on `localhost:50051`
bazel run //bitcode:main

# Starting a publicly available bitcode service.
bazel run //bitcode:main -- -listen 0.0.0.0:50051
```

Running the EESI service:

```bash
# Starting the EESI service listening on `localhost:50052`
bazel run //eesi:main

# Starting a publicly available EESI service.
bazel run //eesi:main -- -listen 0.0.0.0:50052
```

Running the Checker service:

```bash
# Starting the checker service listening on `localhost:50053`
bazel run //checker:main

# Starting a publicly available checker service.
bazel run //checker:main -- -listen 0.0.0.0:50053
```

## Command-line Interface

A command-line interface is provided for interacting with the
services. The [README](./cli/README.md) can be read [here](./cli/README.md)

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

This command will result in a binary `hello.bc` file.

## Service descriptions 
### Bitcode Service

The bitcode service is responsible for managing bitcode files and 
performing operations on them. See `proto/bitcode.proto` for details
of the operations that are available. All of the other services in 
`ErrorSpecifications` depend on the bitcode service.

The bitcode service can be started with `bazel run //bitcode:main`.
This will start the service bound to localhost on port 50051.
All other bazel targets are for internal use only.

### EESI Service

The EESI service infers function error specifications for bitcode files.
See `proto/eesi.proto` for details of the operations that are available.

## Pipeline Diagram

 ![diagram](./EESIER_PIPELINE.png)
