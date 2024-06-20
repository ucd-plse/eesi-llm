"""Handles arguments related to interacting with the EESI service."""

import sys

import glog as log

import cli.common.service_configuration_handler as service_handler
import cli.db.db
import cli.eesi.commands
import cli.eesi.domain_knowledge_handler as dk_handler
import cli.eesi.synonym_configuration_handler as synonym_handler

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
        "--use-embedding",
        action="store_true",
        default=False,
    )
    eesi_get_specifications_all_parser.add_argument(
        "--min-synonym-similarity",
        type=float,
        default=0.7,
        help="Minimum similarity threshold for finding synonym functions."
    )
    eesi_get_specifications_all_parser.add_argument(
        "--min-synonym-evidence",
        type=int,
        default=5,
        help="Minimum number of synonym functions to use for embedding-based"
             " specification inference.",
    )
    eesi_get_specifications_all_parser.add_argument(
        "--expansion-operation",
        choices=["meet", "join", "max"],
        default="meet",
        help="The operation to use during the embedding-guided expansion.",
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
        "--use-embedding",
        action="store_true",
        default=False,
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--min-synonym-similarity",
        type=float,
        default=0.7,
        help="Minimum similarity threshold for finding synonym functions."
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--min-synonym-evidence",
        type=int,
        default=5,
        help="Minimum number of synonym functions to use for embedding-based"
             " specification inference.",
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--expansion-operation",
        choices=["meet", "join", "max"],
        default="meet",
        help="The operation to use during the embedding-guided expansion.",
    )
    eesi_get_specifications_uri_parser.add_argument(
        "--llm",
        choices=["gpt-3.5", "gpt-4.0", "llama-7b", "llama-14b", "llama-34b"],
        default="llama-7b",
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

    # EESI service: GetPredictedSpecificationsUri
    eesi_get_predicted_specifications_uri_parser = eesi_parser.add_parser(
        "GetPredictedSpecificationsUri",
        help="Gets the predicted specifications results for an"
             " individual bitcode file."
    )
    eesi_get_predicted_specifications_uri_parser.add_argument(
        "--bitcode-uri",
        required=True,
        help="Bitcode URI to get predicted specifications for.",
    )
    eesi_get_predicted_specifications_uri_parser.add_argument(
        "--top-k",
        type=int,
        default=3,
        help="The top-k similar functions to look at for predicted "
             "specifications.",
    )

    # EESI service: GetPredictedSpecificationsAll
    eesi_get_predicted_specifications_all_parser = eesi_parser.add_parser(
        "GetPredictedSpecificationsAll",
        help="Gets the predicted specifications results for all bitcode "
             "files registered with your database."
    )
    eesi_get_predicted_specifications_all_parser.add_argument(
        "--top-ks",
        type=int,
        required=True,
        nargs="+",
        help="The top-k (or multiple k's) similar functions to look at for "
             "predicted specifications.",
    )
    eesi_get_predicted_specifications_all_parser.add_argument(
        "--plot",
        help="Flag for plotting the varying precision for projects and top-k's",
        action="store_true",
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

    # EESI service: SpecificationsTableToLatex
    eesi_specifications_table_to_latex_parser = eesi_parser.add_parser(
        "SpecificationsTableToLatex",
        help="Outputs specifications table to latex format.",
    )
    eesi_specifications_table_to_latex_parser.add_argument(
        "--confidence-threshold",
        type=int,
        default=100,
        help="The confidence to threshold lattice elements on.",
    )

    # EESI service: ListSpecificationsTable
    eesi_list_specifications_table_parser = eesi_parser.add_parser(
        "ListSpecificationsTable",
        help="Lists a table for the number of specifications per bitcode file.",
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
        help="Print the row counts in latex format. Note that this only does "
             "the counts in this format, as the other information is not "
             "supplied in the paper.",
     )

    # EESI service: ListSpecificationsCoverage
    eesi_list_specifications_coverage = eesi_parser.add_parser(
        "ListSpecificationsCoverage",
        help="Prints out the coverage report for inferred specifications.",
    )
    eesi_list_specifications_coverage.add_argument(
        "--bitcode-uri",
        default=None,
        help="Bitcode URI to filter for.",
    )

    # EESI service: ListSpecificationsDiff
    eesi_list_specifications_diff_parser = eesi_parser.add_parser(
        "ListSpecificationsDiff",
        help="Lists the specifications diff between two databases",
    )
    eesi_list_specifications_diff_parser.add_argument(
        "--db-1",
        required=True,
    )
    eesi_list_specifications_diff_parser.add_argument(
        "--db-2",
        required=True,
    )
    eesi_list_specifications_diff_parser.add_argument(
        "--confidence-threshold",
        type=int,
        default=100,
        help="The confidence to threshold lattice elements on.",
    )
    eesi_list_specifications_diff_parser.add_argument(
        "--bitcode-uri",
        default=None,
        help="Bitcode URI to filter for.",
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

    # EESI Service; PlotSpecificationsCounts
    eesi_plot_specifications_counts = eesi_parser.add_parser(
        "PlotSpecificationsCounts",
    )
    eesi_plot_specifications_counts.add_argument(
        "--db-meet",
        required=True,
    )
    eesi_plot_specifications_counts.add_argument(
        "--db-join",
        required=True,
    )
    eesi_plot_specifications_counts.add_argument(
        "--db-max",
        required=True,
    )

def parse_args(args):
    """Calls the argument parsing function related to args.rpc."""
    assert args.service.lower() == "eesi"

    parser_function = dict({
        "getspecificationsall": parse_eesi_get_specifications_all_args,
        "getspecificationsuri": parse_eesi_get_specifications_uri_args,
        "injectspecifications": parse_eesi_inject_specifications_args,
        "getpredictedspecificationsuri":
            parse_eesi_get_predicted_specifications_uri_args,
        "getpredictedspecificationsall":
            parse_eesi_get_predicted_specifications_all_args,
        "listspecifications": parse_eesi_list_specifications_args,
        "listspecificationscoverage":
            parse_eesi_list_specifications_coverage_args,
        "listspecificationstable": parse_eesi_list_specifications_table_args,
        "specificationstabletocsv": parse_eesi_specifications_table_to_csv_args,
        "specificationstabletolatex":
            parse_eesi_specifications_table_to_latex_args,
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
        embedding_address=args.embedding_address,
        embedding_port=args.embedding_port,
        max_tasks=args.max_tasks,)
    domain_knowledge_handler = dk_handler.DomainKnowledgeHandler(
        initial_specifications_path=args.initial_specifications,
        error_only_path=args.error_only,
        error_codes_path=args.error_codes,
        success_codes_path=args.success_codes)
    synonym_config_handler = synonym_handler.SynonymConfigurationHandler(
        use_embedding=args.use_embedding,
        minimum_evidence=args.min_synonym_evidence,
        minimum_similarity=args.min_synonym_similarity,
        expansion_operation=args.expansion_operation,)

    command = cli.eesi.commands.get_specifications_all
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_config_handler
    command_kwargs["domain_knowledge_handler"] = domain_knowledge_handler
    command_kwargs["synonym_configuration_handler"] = \
        synonym_config_handler
    command_kwargs["overwrite"] = args.overwrite
    command_kwargs["smart_success_code_zero"] = args.smart_success_code_zero

    return command, command_kwargs
def parse_eesi_get_specifications_uri_args(args):
    """Parses command-line arguments for GetSpecificationsUri"""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_config_handler = service_handler.ServiceConfigurationHandler(
        eesi_address=args.eesi_address, eesi_port=args.eesi_port,
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,
        embedding_address=args.embedding_address,
        embedding_port=args.embedding_port,
        max_tasks=args.max_tasks,)
    domain_knowledge_handler = dk_handler.DomainKnowledgeHandler(
        initial_specifications_path=args.initial_specifications,
        error_only_path=args.error_only,
        error_codes_path=args.error_codes,
        success_codes_path=args.success_codes)
    synonym_config_handler = synonym_handler.SynonymConfigurationHandler(
        use_embedding=args.use_embedding,
        minimum_evidence=args.min_synonym_evidence,
        minimum_similarity=args.min_synonym_similarity,
        expansion_operation=args.expansion_operation,)

    command = cli.eesi.commands.get_specifications_uri
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["bitcode_uri"] = args.bitcode_uri
    command_kwargs["service_configuration_handler"] = \
        service_config_handler
    command_kwargs["domain_knowledge_handler"] = domain_knowledge_handler
    command_kwargs["synonym_configuration_handler"] = \
        synonym_config_handler
    command_kwargs["overwrite"] = args.overwrite
    command_kwargs["smart_success_code_zero"] = args.smart_success_code_zero
    command_kwargs["ctags_file"] = args.ctags

    return command, command_kwargs

def parse_eesi_inject_specifications_args(args):
    """Parses command-line arguments for InjectSpecifications"""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)
    service_config_handler = service_handler.ServiceConfigurationHandler(
        eesi_address=args.eesi_address, eesi_port=args.eesi_port,
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,
        embedding_address=args.embedding_address,
        embedding_port=args.embedding_port,
        max_tasks=args.max_tasks,)
    domain_knowledge_handler = dk_handler.DomainKnowledgeHandler(
        initial_specifications_path=args.initial_specifications,
        error_only_path=args.error_only,
        error_codes_path=args.error_codes,
        success_codes_path=args.success_codes)
    synonym_config_handler = synonym_handler.SynonymConfigurationHandler(
        use_embedding=False,
        minimum_evidence=0,
        minimum_similarity=0,)

    command = cli.eesi.commands.inject_specifications
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["bitcode_uri"] = args.bitcode_uri
    command_kwargs["specifications_path"] = args.specifications
    command_kwargs["service_configuration_handler"] = \
        service_config_handler
    command_kwargs["domain_knowledge_handler"] = domain_knowledge_handler
    command_kwargs["synonym_configuration_handler"] = \
        synonym_config_handler
    command_kwargs["overwrite"] = args.overwrite
    command_kwargs["smart_success_code_zero"] = args.smart_success_code_zero

    return command, command_kwargs

def parse_eesi_get_predicted_specifications_uri_args(args):
    """Parses command-line arguments for GetPredictedSpecificationsUri."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)

    service_config_handler = service_handler.ServiceConfigurationHandler(
        eesi_address=args.eesi_address, eesi_port=args.eesi_port,
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,
        embedding_address=args.embedding_address,
        embedding_port=args.embedding_port,
        max_tasks=args.max_tasks,)

    command = cli.eesi.commands.get_predicted_specifications_uri
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["bitcode_uri"] = args.bitcode_uri
    command_kwargs["service_configuration_handler"] = \
        service_config_handler
    command_kwargs["top_k"] = args.top_k

    return command, command_kwargs

def parse_eesi_get_predicted_specifications_all_args(args):
    """Parses command-line arguments for GetPredictedSpecificationsAll."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)

    service_config_handler = service_handler.ServiceConfigurationHandler(
        eesi_address=args.eesi_address, eesi_port=args.eesi_port,
        bitcode_address=args.bitcode_address, bitcode_port=args.bitcode_port,
        embedding_address=args.embedding_address,
        embedding_port=args.embedding_port,
        max_tasks=args.max_tasks,)

    command = cli.eesi.commands.get_predicted_specifications_all
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["service_configuration_handler"] = \
        service_config_handler
    command_kwargs["top_ks"] = args.top_ks
    command_kwargs["plot"] = args.plot

    return command, command_kwargs

def parse_eesi_list_specifications_coverage_args(args):
    """Parses command-line arguments for ListSpecificationsCoverage."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)

    command = cli.eesi.commands.list_specifications_coverage
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["bitcode_uri_filter"] = args.bitcode_uri

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

def parse_eesi_specifications_table_to_latex_args(args):
    """Parses command-line arguments for ListSpecificationsTable."""

    database = cli.db.db.connect(args.db_name, args.db_host, args.db_port)

    command = cli.eesi.commands.specifications_table_to_latex
    command_kwargs = dict()
    command_kwargs["database"] = database
    command_kwargs["confidence_threshold"] = args.confidence_threshold

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
