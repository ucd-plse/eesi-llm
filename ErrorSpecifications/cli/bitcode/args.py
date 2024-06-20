"""Handles command line arguments related to bitcode services. """

import sys

import glog as log

import cli.bitcode.commands
import cli.common.service_configuration_handler as service_handler
import cli.db.db

def add_arguments(service_parsers):
    """Adds arguments and subparsers for commands related to bitcode services.

    Args:
        service_parsers: Main argparse parser that handles multiple, various
            services.
    """

    # Creating a bitcode service parser to handle all commands related to
    # bitcode services.
    bitcode_service_parsers = service_parsers.add_parser(
        "bitcode",
        help="Commands for RPC calls to the bitcode service",
    )
    bitcode_parser = bitcode_service_parsers.add_subparsers(
        title="command",
        dest="command",
    )
    bitcode_parser.required = True

    ## Bitcode service: RegisterBitcode
    # Registers a single bitcode file from a given uri
    bitcode_register_bitcode_parser = bitcode_parser.add_parser(
        "RegisterBitcode",
        help="RegisterBitcode RPC call",
    )
    bitcode_register_bitcode_parser.add_argument(
        "--uri",
        help="URI of the bitcode file to register. This URI is relative to the "
             "server that the bitcode service is running on.",
        required=True,
    )
    bitcode_register_bitcode_parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Flag for overwriting entries already in database",
        default=False,
    )

    ## Bitcode service: RegisterLocalDataset
    # Registers a dataset of bitcode given a local (file://) uri
    bitcode_register_local_dataset_parser = bitcode_parser.add_parser(
        "RegisterLocalDataset",
        help="Registers bitcode in a local dataset."
    )
    bitcode_register_local_dataset_parser.add_argument(
        "--uri",
        help="URI of the bitcode file to register. This URI is relative to "
             "the server that the bitcode service is running on.",
        required=True,
    )
    bitcode_register_local_dataset_parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Flag for overwriting entries already in database",
        default=False,
    )

    ## Bitcode service: GetCalledFunctionsAll
    # Gets called functions for all bitcode files that are registered
    # and stored in the database that the user provides.
    bitcode_get_called_functions_all_parser = bitcode_parser.add_parser(
        "GetCalledFunctionsAll",
        help="GetCalledFunctions RPC call for all bitcode files in db"
    )
    bitcode_get_called_functions_all_parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Flag for overwriting entries already in database",
        default=False,
    )

    ## Bitcode service: GetCalledFunctionsUri
    # Handles getting all called functions for a single file from a uri
    bitcode_get_called_functions_uri_parser = bitcode_parser.add_parser(
        "GetCalledFunctionsUri",
        help="GetCalledFunctions RPC call for one bitcode file"
    )
    bitcode_get_called_functions_uri_parser.add_argument(
        "--uri",
        help="URI of the bitcode file to get called functions for. This URI"
             " is relative to the server that the bitcode file is running on.",
        required=True,
    )
    bitcode_get_called_functions_uri_parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Flag for overwriting entries already in database",
        default=False,
    )

    ## Bitcode service: GetLocalCalledFunctionsAll
    # Gets local called functions for all bitcode files that are registered
    # and stored in the database that the user provides.
    bitcode_get_local_called_functions_all_parser = bitcode_parser.add_parser(
        "GetLocalCalledFunctionsAll",
        help="GetLocalCalledFunctions RPC call for all bitcode files in db"
    )
    bitcode_get_local_called_functions_all_parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Flag for overwriting entries already in database",
        default=False,
    )

    ## Bitcode service: GetLocalCalledFunctionsUri
    # Handles getting all local called functions for a single file from a uri
    bitcode_get_local_called_functions_uri_parser = bitcode_parser.add_parser(
        "GetLocalCalledFunctionsUri",
        help="GetLocalCalledFunctions RPC call for one bitcode file"
    )
    bitcode_get_local_called_functions_uri_parser.add_argument(
        "--uri",
        help="URI of the bitcode file to get local called functions for. This"
             " URI is relative to the server that the bitcode file is running"
             " on.",
        required=True,
    )
    bitcode_get_local_called_functions_uri_parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Flag for overwriting entries already in database",
        default=False,
    )

    ## Bitcode service: GetFileCalledFunctionsAll
    # Gets file called functions for all bitcode files that are registered
    # and stored in the database that the user provides.
    bitcode_get_file_called_functions_all_parser = bitcode_parser.add_parser(
        "GetFileCalledFunctionsAll",
        help="GetFileCalledFunctions RPC call for all bitcode files in db"
    )
    bitcode_get_file_called_functions_all_parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Flag for overwriting entries already in database",
        default=False,
    )

    ## Bitcode service: GetFileCalledFunctionsUri
    # Handles getting all file called functions for a single file from a uri
    bitcode_get_file_called_functions_uri_parser = bitcode_parser.add_parser(
        "GetFileCalledFunctionsUri",
        help="GetFileCalledFunctions RPC call for one bitcode file"
    )
    bitcode_get_file_called_functions_uri_parser.add_argument(
        "--uri",
        help="URI of the bitcode file to get file called functions for. This"
             " URI is relative to the server that the bitcode file is running"
             " on.",
        required=True,
    )
    bitcode_get_file_called_functions_uri_parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Flag for overwriting entries already in database",
        default=False,
    )

    ## Bitcode service: GetDefinedFunctionsAll
    # Gets defined functions for all bitcode files that are registered
    # and stored in the database that the user provides.
    bitcode_get_defined_functions_all_parser = bitcode_parser.add_parser(
        "GetDefinedFunctionsAll",
        help="GetDefinedFunctions RPC call for all bitcode files in db"
    )
    bitcode_get_defined_functions_all_parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Flag for overwriting entries already in database",
        default=False,
    )

    ## Bitcode service: GetDefinedFunctionsUri
    # Handles getting all defined functions for a single file form a uri
    bitcode_get_defined_functions_uri_parser = bitcode_parser.add_parser(
        "GetDefinedFunctionsUri",
        help="GetDefinedFunctions RPC call for one bitcode file"
    )
    bitcode_get_defined_functions_uri_parser.add_argument(
        "--uri",
        help="URI of the bitcode file to get defined functions for. This URI"
             " is relative to the server that the bitcode file is running on.",
    )
    bitcode_get_defined_functions_uri_parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Flag for overwriting entries already in database",
        default=False,
    )

    ## Bitcode serve: ListRegisteredBitcode
    # Lists all registered bitcode in MongoDB.
    bitcode_list_registered_bitcode_parser = bitcode_parser.add_parser(
        "ListRegisteredBitcode",
        help="Lists all registered bitcode in MongoDB."
    )

    bitcode_list_called_functions_parser = bitcode_parser.add_parser(
        "ListCalledFunctions",
        help="Lists all called functions in MongoDB."
    )

    bitcode_list_defined_functions_parser = bitcode_parser.add_parser(
        "ListDefinedFunctions",
        help="Lists all defined functions in MongoDB."
    )


def parse_args(args):
    """Determines command user called and parses arguments accordingly."""

    assert(args.service.lower() == "bitcode")

    parser_function = dict({
        "registerbitcode": parse_bitcode_register_bitcode_args,
        "getcalledfunctionsall": parse_bitcode_get_called_functions_all_args,
        "getcalledfunctionsuri": parse_bitcode_get_called_functions_uri_args,
        "getlocalcalledfunctionsall":
            parse_bitcode_get_local_called_functions_all_args,
        "getlocalcalledfunctionsuri":
            parse_bitcode_get_local_called_functions_uri_args,
        "getfilecalledfunctionsall":
            parse_bitcode_get_file_called_functions_all_args,
        "getfilecalledfunctionsuri":
            parse_bitcode_get_file_called_functions_uri_args,
        "getdefinedfunctionsall": parse_bitcode_get_defined_functions_all_args,
        "getdefinedfunctionsuri": parse_bitcode_get_defined_functions_uri_args,
        "registerlocaldataset": parse_bitcode_register_local_dataset_args,
        "listregisteredbitcode": parse_bitcode_list_registered_bitcode_args,
        "listcalledfunctions": parse_bitcode_list_called_functions_args,
        "listdefinedfunctions": parse_bitcode_list_defined_functions_args,
    })
    try:
        return parser_function[args.command.lower()](args)
    except KeyError:
        log.error("Incorrect command {} provided!".format(args.command))
        sys.exit(1)

def parse_bitcode_register_bitcode_args(args):
    """ Parses arguments for RegisterBitcode service."""
    # Connect to the database.
    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,)

    command = cli.bitcode.commands.register_bitcode
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_configuration_handler
    command_kwargs["uri"] = args.uri
    command_kwargs["overwrite"] = args.overwrite

    return command, command_kwargs

def parse_bitcode_register_local_dataset_args(args):
    """ Parses arguments for RegisterLocalDataset service."""
    #Connect to the database.
    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,)

    command = cli.bitcode.commands.register_local_dataset
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_configuration_handler
    command_kwargs["uri"] = args.uri
    command_kwargs["overwrite"] = args.overwrite

    return command, command_kwargs

def parse_bitcode_get_called_functions_all_args(args):
    """ Parses arguments for GetCalledFunctionsAll service."""

    # Connect to the database.
    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,
        max_tasks=args.max_tasks)

    command = cli.bitcode.commands.get_called_functions_all
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_configuration_handler
    command_kwargs["overwrite"] = args.overwrite

    return command, command_kwargs

def parse_bitcode_get_called_functions_uri_args(args):
    """ Parses arguments for GetCalledFunctionsUri service."""

    # Connect to the database.
    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,)

    command = cli.bitcode.commands.get_called_functions_uri
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_configuration_handler
    command_kwargs["uri"] = args.uri
    command_kwargs["overwrite"] = args.overwrite

    return command, command_kwargs

def parse_bitcode_get_local_called_functions_all_args(args):
    """Parses arguments for GetLocalCalledFunctionsAll service."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,)

    command = cli.bitcode.commands.get_local_called_functions_all
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_configuration_handler
    command_kwargs["overwrite"] = args.overwrite

    return command, command_kwargs

def parse_bitcode_get_local_called_functions_uri_args(args):
    """Parses arguments for GetLocalCalledFunctionsUri service."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,)

    command = cli.bitcode.commands.get_local_called_functions_uri
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_configuration_handler
    command_kwargs["uri"] = args.uri
    command_kwargs["overwrite"] = args.overwrite

    return command, command_kwargs

def parse_bitcode_get_file_called_functions_all_args(args):
    """Parses arguments for GetFileCalledFunctionsAll service."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,)

    command = cli.bitcode.commands.get_file_called_functions_all
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_configuration_handler
    command_kwargs["overwrite"] = args.overwrite

    return command, command_kwargs

def parse_bitcode_get_file_called_functions_uri_args(args):
    """Parses arguments for GetFileCalledFunctionsUri service."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,)

    command = cli.bitcode.commands.get_file_called_functions_uri
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_configuration_handler
    command_kwargs["uri"] = args.uri
    command_kwargs["overwrite"] = args.overwrite

    return command, command_kwargs

def parse_bitcode_get_defined_functions_all_args(args):
    """ Parses arguments for GetDefinedFunctionsAll service."""

    # Connect to the database.
    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,
        max_tasks=args.max_tasks)

    command = cli.bitcode.commands.get_defined_functions_all
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_configuration_handler
    command_kwargs["overwrite"] = args.overwrite

    return command, command_kwargs

def parse_bitcode_get_defined_functions_uri_args(args):
    """ Parses arguments for GetDefinedFunctionsUri service."""

    # Connect to the database.
    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,)

    command = cli.bitcode.commands.get_defined_functions_uri
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_configuration_handler
    command_kwargs["uri"] = args.uri
    command_kwargs["overwrite"] = args.overwrite

    return command, command_kwargs

def parse_bitcode_list_registered_bitcode_args(args):
    """Parses arguments for ListRegisteredBitcode."""

    # Connect to the database.
    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)

    command = cli.bitcode.commands.list_registered_bitcode
    command_kwargs = dict()
    command_kwargs["database"] = database

    return command, command_kwargs

def parse_bitcode_list_called_functions_args(args):
    """Parses arguments for ListCalledFunctions."""
    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)

    command = cli.bitcode.commands.list_called_functions
    command_kwargs = dict()
    command_kwargs["database"] = database

    return command, command_kwargs

def parse_bitcode_list_defined_functions_args(args):
    """Parses arguments for ListDefinedFunctions."""
    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)

    command = cli.bitcode.commands.list_defined_functions
    command_kwargs = dict()
    command_kwargs["database"] = database

    return command, command_kwargs
