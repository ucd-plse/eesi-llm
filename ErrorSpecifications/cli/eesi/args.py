"""Handles arguments related to interacting with the EESI service."""

import sys

import glog as log

import cli.common.service_configuration_handler as service_handler
import cli.db.db
import cli.eesi.commands
import cli.eesi.domain_knowledge_handler as dk_handler

def add_arguments(service_parsers):
    """Adds all command-line arguments for EESI to main CLI service parser."""
    # EESI service
    eesi_service_parsers = service_parsers.add_parser(
        "eesi",
        help="Commands for RPC calls to the eesi service",
    )
    eesi_parser = eesi_service_parsers.add_subparsers(
        title="command", dest='command')
    eesi_parser.required = True

    ## EESI service: GetSpecificationsAll
    eesi_get_specifications_all_parser = eesi_parser.add_parser(
        "GetSpecificationsAll", help='GetSpecifications RPC call')
    eesi_get_specifications_all_parser.add_argument(
        "--initial-specifications",
        default=None,
        help="Absolute path to initial-specifications file",
    )
    eesi_get_specifications_all_parser.add_argument(
        "--error-only",
        default=None,
        help="Absolute path to the error only file"
    )
    eesi_get_specifications_all_parser.add_argument(
        "--error-codes",
        default=None,
        help="Absolute path to the error codes file"
    )
    eesi_get_specifications_all_parser.add_argument(
        "--success-codes",
        default=None,
        help="Absolute path to the success codes file"
    )
    eesi_get_specifications_all_parser.add_argument(
        "--smart-success-code-zero",
        action="store_true",
        default=False,
        help="Whether to use a heuristic to determine if 0 is a success code"
    )
    eesi_get_specifications_all_parser.add_argument(
        "--bitcode-uri",
        default=None,
        help="URI of bitcode file"
    )
    eesi_get_specifications_all_parser.add_argument(
        "--overwrite",
        action="store_true",
        default=False,
    )
    eesi_get_specifications_all_parser.add_argument(
        "--llm-name",
        default="gpt-4.1-mini-2025-04-14",
        help="The LLM to use for expansion.",
    )

    ## EESI service: GetSpecificationsUri
    eesi_get_specifications_uri_parser = eesi_parser.add_parser(
        "GetSpecificationsUri",
        help="GetSpecifications RPC call for a single bitcode file from URI",
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--initial-specifications",
        default=None,
        help="Absolute path to initial-specifications file"
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--error-only",
        default=None,
        help="Absolute path to the error only file"
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--error-codes",
        default=None,
        help="Absolute path to the error codes file",
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--success-codes",
        default=None,
        help="Absolute path to the success codes file"
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--smart-success-code-zero",
        action="store_true",
        default=False,
        help="Whether to use a heuristic to determine if 0 is a success code"
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--bitcode-uri",
        default=None,
        required=True,
        help="URI of bitcode file.",
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--ctags",
        default=None,
        help="The path to the Ctags file for the benchmark analyzed.",
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--overwrite",
        action="store_true",
        default=False,
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--llm-name",
        default="gpt-4.1-mini-2025-04-14",
        help="The LLM to use for expansion.",
    )

    ## EESI service: InjectSpecifications 
    eesi_inject_specifications_parser = eesi_parser.add_parser(
        "InjectSpecifications",
        help="Inject specifications for a bitcode URI given a list of "
             "specifications in a file.",
    )
    eesi_inject_specifications_parser.add_argument(
        "--initial-specifications",
        default=None,
        help="Absolute path to initial-specifications file."
    )
    eesi_inject_specifications_parser.add_argument(
        "--specifications",
        help="Absolute path to specifications file.",
        required=True,
    )
    eesi_inject_specifications_parser.add_argument(
        "--error-only",
        default=None,
        help="Absolute path to the error only file"
    )
    eesi_inject_specifications_parser.add_argument(
        "--error-codes",
        default=None,
        help="Absolute path to the error codes file",
    )
    eesi_inject_specifications_parser.add_argument(
        "--success-codes",
        default=None,
        help="Absolute path to the success codes file"
    )
    eesi_inject_specifications_parser.add_argument(
        "--smart-success-code-zero",
        action="store_true",
        default=False,
        help="Whether to use a heuristic to determine if 0 is a success code"
    )
    eesi_inject_specifications_parser.add_argument(
        "--bitcode-uri",
        default=None,
        required=True,
        help="URI of bitcode file.",
    )
    eesi_inject_specifications_parser.add_argument(
        "--overwrite",
        action="store_true",
        default=False,
    )

    # EESI service: ListSpecifications
    eesi_list_specifications_parser = eesi_parser.add_parser(
        "ListSpecifications",
        help="Lists specifications for bitcode files in MongoDB."
    )
    eesi_list_specifications_parser.add_argument(
        "--bitcode-uri",
        default=None,
        help="Bitcode URI to filter for.",
    )
    eesi_list_specifications_parser.add_argument(
        "--confidence-threshold",
        type=int,
        default=100,
        help="The confidence to threshold lattice elements on.",
    )
    eesi_list_specifications_parser.add_argument(
        "--raw",
        action="store_true",
        default=False,
    )


    # EESI service: SpecificationsTableToCsv
    eesi_specifications_table_to_csv_parser = eesi_parser.add_parser(
        "SpecificationsTableToCsv",
        help="Outputs specifications table to a csv file.",
    )
    eesi_specifications_table_to_csv_parser.add_argument(
        "--confidence-threshold",
        type=int,
        default=100,
        help="The confidence to threshold lattice elements on.",
    )
    eesi_specifications_table_to_csv_parser.add_argument(
        "--bitcode-uri",
        default=None,
        help="Bitcode URI to filter for.",
    )
    eesi_specifications_table_to_csv_parser.add_argument(
        "--output-path",
        required=True,
        help="Output path where .csv file will be written.",
    )
    # EESI service: ListSpecificationsTable 
    eesi_list_specifications_table_parser = eesi_parser.add_parser(
        "ListSpecificationsTable",
        help="Outputs specifications table to a csv file.",
    )
    eesi_list_specifications_table_parser.add_argument(
        "--confidence-threshold",
        type=int,
        default=100,
        help="The confidence to threshold lattice elements on.",
    )
    eesi_list_specifications_table_parser.add_argument(
        "--bitcode-uri",
        default=None,
        help="Bitcode URI to filter for.",
    )
    eesi_list_specifications_table_parser.add_argument(
        "--to-latex",
        action="store_true",
        default=False,
    )



    # EESI service: ListStatisticsDatabase
    eesi_list_statistics_database_args = eesi_parser.add_parser(
        "ListStatisticsDatabase",
        help="Lists the statistics against ground truth for specifications"
             "stored in the database.",
    )
    eesi_list_statistics_database_args.add_argument(
        "--ground-truth-path",
        help="Path to ground-truth file.",
        required=True,

    )
    eesi_list_statistics_database_args.add_argument(
        "--bitcode-uri",
        default=None,
        help="Bitcode URI to filter for.",
        required=True,
    )
    eesi_list_statistics_database_args.add_argument(
        "--confidence-threshold",
        default=100,
        type=int,
        help="Confidence threshold for specifications.",
    )

    # EESI service: ListStatisticsFile
    eesi_list_statistics_file_args = eesi_parser.add_parser(
        "ListStatisticsFile",
        help="Lists the statistics against ground truth for specifications"
             " written out to file.",
    )
    eesi_list_statistics_file_args.add_argument(
        "--ground-truth-path",
        help="The path to the specifications ground-truth file.",
        required=True,

    )
    eesi_list_statistics_file_args.add_argument(
        "--specifications-path",
        help="The path to the specifications file.",
        required=True,
    )
    eesi_list_statistics_file_args.add_argument(
        "--initial-specifications-path",
        help="The path to the initial specifications file.",
        default=None,
    )
    eesi_list_statistics_file_args.add_argument(
        "--confidence-threshold",
        default=100,
        type=int,
        help="Confidence threshold for specifications.",
    )

def parse_args(args):
    """Calls the argument parsing function related to args.rpc."""
    assert args.service.lower() == "eesi"

    parser_function = dict({
        "getspecificationsall": parse_eesi_get_specifications_all_args,
        "getspecificationsuri": parse_eesi_get_specifications_uri_args,
        "injectspecifications": parse_eesi_inject_specifications_args,
        "listspecifications": parse_eesi_list_specifications_args,
        "listspecificationstable": parse_eesi_list_specifications_table_args,
        "specificationstabletocsv": parse_eesi_specifications_table_to_csv_args,
        "listspecificationsdiff": parse_eesi_list_specifications_diff_args,
        "liststatisticsdatabase": parse_eesi_list_statistics_database_args,
        "liststatisticsfile": parse_eesi_list_statistics_file_args,
        "plotspecificationscounts": parse_eesi_plot_specifications_counts,
    })

    try:
        return parser_function[args.command.lower()](args)
    except KeyError:
        log.error("Incorrect command {} provided!".format(args.command))
        sys.exit(1)

def parse_eesi_get_specifications_all_args(args):
    """Parses command-line arguments for GetSpecificationsAll."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_config_handler = service_handler.ServiceConfigurationHandler(
        eesi_address=args.eesi_address, eesi_port=args.eesi_port,
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,
        max_tasks=args.max_tasks,)
    domain_knowledge_handler = dk_handler.DomainKnowledgeHandler(
        initial_specifications_path=args.initial_specifications,
        error_only_path=args.error_only,
        error_codes_path=args.error_codes,
        success_codes_path=args.success_codes)

    command = cli.eesi.commands.get_specifications_all
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_config_handler
    command_kwargs["domain_knowledge_handler"] = domain_knowledge_handler
    command_kwargs["overwrite"] = args.overwrite
    command_kwargs["smart_success_code_zero"] = args.smart_success_code_zero
    command_kwargs["llm_name"] = args.llm_name

    return command, command_kwargs
def parse_eesi_get_specifications_uri_args(args):
    """Parses command-line arguments for GetSpecificationsUri"""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_config_handler = service_handler.ServiceConfigurationHandler(
        eesi_address=args.eesi_address, eesi_port=args.eesi_port,
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,
        max_tasks=args.max_tasks,)
    domain_knowledge_handler = dk_handler.DomainKnowledgeHandler(
        initial_specifications_path=args.initial_specifications,
        error_only_path=args.error_only,
        error_codes_path=args.error_codes,
        success_codes_path=args.success_codes)

    command = cli.eesi.commands.get_specifications_uri
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["bitcode_uri"] = args.bitcode_uri
    command_kwargs["service_configuration_handler"] = \
        service_config_handler
    command_kwargs["domain_knowledge_handler"] = domain_knowledge_handler
    command_kwargs["overwrite"] = args.overwrite
    command_kwargs["smart_success_code_zero"] = args.smart_success_code_zero
    command_kwargs["ctags_file"] = args.ctags
    command_kwargs["llm_name"] = args.llm_name

    return command, command_kwargs

def parse_eesi_inject_specifications_args(args):
    """Parses command-line arguments for InjectSpecifications"""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_config_handler = service_handler.ServiceConfigurationHandler(
        eesi_address=args.eesi_address, eesi_port=args.eesi_port,
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,
        max_tasks=args.max_tasks,)
    domain_knowledge_handler = dk_handler.DomainKnowledgeHandler(
        initial_specifications_path=args.initial_specifications,
        error_only_path=args.error_only,
        error_codes_path=args.error_codes,
        success_codes_path=args.success_codes)

    command = cli.eesi.commands.inject_specifications
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["bitcode_uri"] = args.bitcode_uri
    command_kwargs["specifications_path"] = args.specifications
    command_kwargs["service_configuration_handler"] = \
        service_config_handler
    command_kwargs["domain_knowledge_handler"] = domain_knowledge_handler
    command_kwargs["overwrite"] = args.overwrite
    command_kwargs["smart_success_code_zero"] = args.smart_success_code_zero

    return command, command_kwargs

def parse_eesi_list_specifications_args(args):
    """Parses command-line arguments for ListSpecifications."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)

    command = cli.eesi.commands.list_specifications
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["bitcode_uri_filter"] = args.bitcode_uri
    command_kwargs["confidence_threshold"] = args.confidence_threshold
    command_kwargs["raw"] = args.raw

    return command, command_kwargs

def parse_eesi_list_specifications_table_args(args):
    """Parses command-line arguments for ListSpecificationsTable."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)

    command = cli.eesi.commands.list_specifications_table
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["bitcode_uri_filter"] = args.bitcode_uri
    command_kwargs["confidence_threshold"] = args.confidence_threshold
    command_kwargs["to_latex"] = args.to_latex

    return command, command_kwargs

def parse_eesi_specifications_table_to_csv_args(args):
    """Parses command-line arguments for SpecificationsTableToCsv."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)

    command = cli.eesi.commands.specifications_table_to_csv
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["bitcode_uri_filter"] = args.bitcode_uri
    command_kwargs["confidence_threshold"] = args.confidence_threshold
    command_kwargs["output_path"] = args.output_path

    return command, command_kwargs

def parse_eesi_list_specifications_diff_args(args):
    """Parses command-line arguments for ListSpecificationsDiff."""

    database1 = cli.db.db.connect(args.db_1, args.db_host, args.db_port)
    database2 = cli.db.db.connect(args.db_2, args.db_host, args.db_port)

    command = cli.eesi.commands.list_specifications_diff
    command_kwargs = dict()
    command_kwargs["database_a"] = database1
    command_kwargs["database_b"] = database2
    command_kwargs["bitcode_uri_filter"] = args.bitcode_uri
    command_kwargs["confidence_threshold"] = args.confidence_threshold

    return command, command_kwargs

def parse_eesi_list_statistics_database_args(args):
    """Parses command-line arguments for ListStatisticsDatabase."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)

    command = cli.eesi.commands.list_statistics_database
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["bitcode_uri"] = args.bitcode_uri
    command_kwargs["ground_truth_file"] = args.ground_truth_path
    command_kwargs["t"] = args.confidence_threshold

    return command, command_kwargs

def parse_eesi_list_statistics_file_args(args):
    """Parses command-line arguments for ListStatisticsFile."""

    command = cli.eesi.commands.list_statistics_file
    command_kwargs = dict()
    command_kwargs["ground_truth_file"] = args.ground_truth_path
    command_kwargs["specs_path"] = args.specifications_path
    command_kwargs["init_specs_path"] = args.initial_specifications_path
    command_kwargs["t"] = args.confidence_threshold

    return command, command_kwargs

def parse_eesi_plot_specifications_counts(args):
    """Parses command-line arguments for PlotSpecificationsCounts."""
    database_meet = cli.db.db.connect(args.db_meet, args.db_host, args.db_port)
    database_join = cli.db.db.connect(args.db_join, args.db_host, args.db_port)
    database_max = cli.db.db.connect(args.db_max, args.db_host, args.db_port)

    command = cli.eesi.commands.plot_specifications_counts
    command_kwargs = dict()
    command_kwargs["database_meet"] = database_meet 
    command_kwargs["database_join"] = database_join
    command_kwargs["database_max"] = database_max

    return command, command_kwargs
