# Command-line Interface

## Testing and Building
### Building and Dependencies

See [ErrorSpecifications/README.md](../README.md).

### Running Tests

To run the provided **CLI** tests, use the command:

```bash
bazel test //cli/...
```

These tests are located in CLI test [directory](./test/).

## Before Running

When running the CLI, the relevant services must also be running.
For example, when using commands related to the bitcode service,
the bitcode service must be launched and actively listening as 
demonstrated in [ErrorSpecifications/README.md](../README.md).

## Running

There are several, optional arguments related to non-default configurations. 
For example, running the bitcode service on a non-default port. These arguments
must be specified **before** the related service command, e.g. `bitcode`.

```bash
usage: bazel run //cli:main -- [-h] [--db-address DB_ADDRESS] [--db-port DB_PORT]
                                    [--db-name DB_NAME] [--bitcode-address BITCODE_ADDRESS]
                                    [--bitcode-port BITCODE_PORT] [--eesi-server EESI_SERVER]
                                    [--eesi-port EESI_PORT] [--checker-address CHECKER_ADDRESS]
                                    [--checker-port CHECKER_PORT] [--max-tasks MAX_TASKS]
                                    {bitcode,eesi} ...

optional arguments:
  -h, --help            show this help message and exit
  --db-address DB_ADDRESS
                        Address where MongoDB is running.
  --db-port DB_PORT     Port number where MongoDB is running.
  --db-name DB_NAME     Name of database to store analysis results.
  --bitcode-address BITCODE_ADDRESS
                        Address where the bitcode service is running.
  --bitcode-port BITCODE_PORT
                        Port number where the bitcode service is running.
  --eesi-server EESI_SERVER
                        Address where the EESI service is running.
  --eesi-port EESI_PORT
                        Port number where the EESI service is running.
  --checker-address CHECKER_ADDRESS
                        Address where the checker service is running.
  --checker-port CHECKER_PORT
                        Port number where the checker service is running.
  --max-tasks MAX_TASKS
                        Maximum number of tasks(requests) to send to a service
                        at once.

service:
  {bitcode,eesi}
    bitcode             Commands for RPC calls to the bitcode service
    eesi                Commands for RPC calls to the eesi service
```

## Commands Related to Services

### Bitcode

The CLI Bitcode commands are related to the process of registering bitcode, 
getting called functions, and getting defined functions. The individual 
commands related to the CLI Bitcode commands can be found in the 
[ErrorSpecifications/cli/bitcode/README.md](./bitcode/README.md).

### EESI

The CLI EESI commands are only related to specifications of bitcode files.
The commands related to getting specifications can be found in
[ErrorSpecifications/cli/eesi/README.md](./eesi/README.md).

### Checker

The CLI Checker commands are only related towards listing the results of
violations that are found by EESIER. Previously, violations were found
separately in the checker service. The checker is no longer a separate service
and violations are found in-between finding error specifications. The command
related to the checker violations is found in
[ErrorSpecifications/cli/checker/README.md](./checker/README.md).
