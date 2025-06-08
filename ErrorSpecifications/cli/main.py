"""Main CLI """

import argparse
import glog as log

import cli.bitcode.args
import cli.eesi.args

def main():
    """Parses the command-line arguments and executes supplied command."""
    log.setLevel("INFO")
    command, command_kwargs = parse_args()
    command(**command_kwargs)

def parse_args():
    """Adds parser arguments and subparsers for CLI commands."""
    # CLI structure is cli.py service command [args]

    # Arguments here are common to all services.
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--db-host",
        default="127.0.0.1",
        help="Address where MongoDB is running.",
    )
    parser.add_argument(
        "--db-port",
        default=27017,
        type=int,
        help="Port number where MongoDB is running.",
    )
    parser.add_argument(
        "--db-name",
        default="error_specifications",
        help="Name of database to store analysis results."
    )
    parser.add_argument(
        "--bitcode-address",
        default="localhost",
        help="Address where the bitcode service is running.",
    )
    parser.add_argument(
        "--bitcode-port",
        default="50051",
        help="Port number where the bitcode service is running.",
    )
    parser.add_argument(
        "--eesi-address",
        default="localhost",
        help="Address where the EESI service is running.",
    )
    parser.add_argument(
        "--eesi-port",
        default="50052",
        help="Port number where the EESI service is running.",
    )
    parser.add_argument(
        "--checker-address",
        default="127.0.0.1",
        help="Address where the checker service is running.",
    )
    parser.add_argument(
        "--checker-port",
        default="50053",
        help="Port number where the checker service is running.",
    )
    parser.add_argument(
        "--max-tasks",
        default=4,
        type=int,
        help="Maximum number of tasks(requests) to send to a service at once.",
    )
    service_parsers = parser.add_subparsers(title="service", dest="service")
    service_parsers.required = True

    cli.bitcode.args.add_arguments(service_parsers)
    cli.eesi.args.add_arguments(service_parsers)

    args = parser.parse_args()

    service_parser_functions = {
        "bitcode": cli.bitcode.args.parse_args,
        "eesi": cli.eesi.args.parse_args,
    }

    service_name = args.service.lower()
    if service_name not in service_parser_functions:
        raise ValueError("Invalid service name.")

    return service_parser_functions[service_name](args)

if __name__ == "__main__":
    main()
