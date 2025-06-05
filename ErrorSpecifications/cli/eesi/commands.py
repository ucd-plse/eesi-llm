"""Commands related to interacting with the EESI service.

The major interaction that occurs with the EESI service is getting
specifications. The two primary functionalities that a user is concerned
with in this module is getting specifications for an invidivual bitcode
file and getting specifications for all registered bitcode files in MongoDB.
"""

from collections import defaultdict
import os

import glog as log
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np
from termcolor import colored

import cli.db.db
import cli.common.log
import cli.common.uri
import cli.bitcode.rpc
import cli.bitcode.commands
import cli.bitcode.db
import cli.eesi.db
import cli.eesi.lattice_helper
import cli.eesi.rpc
import proto.bitcode_pb2
import proto.eesi_pb2

ID_TYPE = "request.bitcodeId.id"
COLLECTION_TYPE = "GetSpecificationsResponse"
LATTICE_ELEMENT_TO_STRING = {
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_BOTTOM: "bottom",
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO: "<0",
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO:
        ">0",
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_ZERO: "==0",
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO:
        "<=0",
    proto.eesi_pb2.SignLatticeElement \
        .SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO: ">=0",
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_NOT_ZERO: "!=0",
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_TOP: "top",
    "emptyset": "emptyset",
}
RETURN_TYPE_TO_STRING = {
    proto.bitcode_pb2.FunctionReturnType.FUNCTION_RETURN_TYPE_INVALID:\
        "INVALID",
    proto.bitcode_pb2.FunctionReturnType.FUNCTION_RETURN_TYPE_VOID: "VOID",
    proto.bitcode_pb2.FunctionReturnType.FUNCTION_RETURN_TYPE_INTEGER:\
        "INTEGER",
    proto.bitcode_pb2.FunctionReturnType.FUNCTION_RETURN_TYPE_POINTER:\
        "POINTER",
    proto.bitcode_pb2.FunctionReturnType.FUNCTION_RETURN_TYPE_OTHER: "OTHER",
}
COMMON_FILES_TO_PROJECT_NAME = defaultdict(str, {
    "mbedtls-reg2mem.bc": "Mbed TLS",
    "pidgin-reg2mem.bc": "Pidgin",
    "openssl-patched-reg2mem.bc": "OpenSSL",
    "linux-reg2mem.bc": "Linux",
    "linux-fsmm-reg2mem.bc": "Linux FS",
    "linux-nfc-reg2mem.bc": "Linux NFC",
    "zlib-reg2mem.bc": "zlib",
    "netdata-reg2mem.bc": "Netdata",
    "lfs-reg2mem.bc": "Little FS",
})

def _setup_called_functions(database, service_configuration_handler,
                            bitcode_id, str_uri):
    log.warning(f"Missing called function entries for {str_uri}. Running"
                " GetCalledFunctions beforehand.")
    cli.bitcode.commands.get_called_functions_uri(
        database, service_configuration_handler, str_uri, False)
    called_functions_response = cli.bitcode.db.read_called_functions_response(
        database, bitcode_id)
    if not called_functions_response:
        log.error(f"{str_uri} still has missing called functions entries."
                  " Please check for errors with the bitcode service.")
        return None

    return called_functions_response

def _setup_defined_functions(database, service_configuration_handler,
                             bitcode_id, str_uri):
    log.warning(f"Missing defined function entries for {str_uri}. Running"
                " GetDefinedFunctions beforehand.")
    cli.bitcode.commands.get_defined_functions_uri(
        database, service_configuration_handler, str_uri, False)
    defined_functions_response = cli.bitcode.db.read_defined_functions_response(
        database, bitcode_id)
    if not defined_functions_response:
        log.error(f"{str_uri} still has missing defined functions entries."
                  " Please check for errors with the bitcode service.")
        return None

    return defined_functions_response

def _re_register_bitcode(database, service_configuration_handler, str_uri):
    """Just calls the bitcode CLI's _re_register_bitcode."""
    return cli.bitcode.commands.re_register_bitcode(
        database, service_configuration_handler, str_uri, False)

def _setup_bitcode(database, service_configuration_handler, uri):
    """Registers the bitcode file and gets called/defined functions."""
    str_uri = cli.common.uri.to_str(uri)
    bitcode_id = _re_register_bitcode(database, service_configuration_handler,
                                      uri)
    called_functions_response = _setup_called_functions(
        database, service_configuration_handler, bitcode_id, str_uri),
    defined_functions_response = _setup_defined_functions(
        database, service_configuration_handler, bitcode_id, str_uri)

    return bitcode_id, called_functions_response, defined_functions_response

def inject_specifications(database, specifications_path,
                              service_configuration_handler,
                              domain_knowledge_handler,
                              smart_success_code_zero,
                              bitcode_uri, overwrite,):
    """Injects a GetSpecificationsResponse given a specifications file.

    Args:
        database: pymongo database object.
    """

    uri = cli.common.uri.parse(bitcode_uri)
    found_id = cli.bitcode.db.read_id_for_uri(database, uri)
    called_functions_response = None
    defined_functions_response = None
    if not found_id:
        found_id, called_functions_response, defined_functions_response, *_ = \
            _setup_bitcode(database, service_configuration_handler, uri)
        if not found_id:
            raise LookupError(f"Bitcode file: {uri.path} is not registering "
                              "with the bitcode service. Please check the "
                              "service log messages for any errors!")

    # Getting the bitcode handle.
    bitcode_id_handle = cli.bitcode.rpc.register_bitcode(
        service_configuration_handler.get_bitcode_stub(), uri)
    bitcode_id_handle.authority = \
        service_configuration_handler.bitcode_config.get_full_address()

    # Deleting current specifications in MongoDB
    # if they exist and overwrite is True.
    if overwrite:
        cli.eesi.db.delete_specifications(
            database=database,
            bitcode_id=bitcode_id_handle.id,
        )

    entry_exists = cli.db.db.collection_contains_id(
        database=database,
        id_type=ID_TYPE,
        unique_id=bitcode_id_handle.id,
        collection=COLLECTION_TYPE,
    )

    if entry_exists:
        log.info(f"Collection {COLLECTION_TYPE} already has an entry for "
                 f"bitcode id {bitcode_id_handle.id}.")
        cli.common.log.log_finished("GetSpecificationsUri", False)
        return

    # Getting called functions to evaluate which specifications
    # from domain knowledge are applicable to a given bitcode file. If
    # this bitcode file was not originally already registered. We should expect
    # to already have these responses
    if not called_functions_response:
        called_functions_response = \
            cli.bitcode.db.read_called_functions_response(
                database, bitcode_id_handle.id)
        if not called_functions_response:
            called_functions_response = _setup_called_functions(
                database, service_configuration_handler, bitcode_id_handle.id,
                bitcode_uri)
    if not defined_functions_response:
        defined_functions_response = \
            cli.bitcode.db.read_defined_functions_response(
                database, bitcode_id_handle.id)
        if not defined_functions_response:
            defined_functions_response = _setup_defined_functions(
                database, service_configuration_handler, bitcode_id_handle.id,
                bitcode_uri)

    # The database must contain entries for both defined and called functions.
    # If either of these conditions hold, then that means something has gone
    # wrong and you should check the logs.
    if not called_functions_response or not defined_functions_response:
        return

    # Using called functions and defined functions to get the
    # applicable initial specifications.
    applicable_initial_specifications = \
        domain_knowledge_handler.get_applicable_specifications(
            called_functions_response, defined_functions_response)

    # Generating a request here instead of eesi.rpc because of the need to
    # match the applicable initial specifications with the individual
    # bitcode file (this is primarily for get_specifications_all).
    request = proto.eesi_pb2.GetSpecificationsRequest(
        bitcode_id=bitcode_id_handle,
        error_only_functions=domain_knowledge_handler.error_only,
        initial_specifications=applicable_initial_specifications,
        error_codes=domain_knowledge_handler.error_codes,
        success_codes=domain_knowledge_handler.success_codes,
        llm_name="Injected",
        smart_success_code_zero=smart_success_code_zero,
    )

    # Parsing the initial specifications text file into lines.
    with open(specifications_path, "r") as specifications_file:
        specifications_lines = [
            x.strip() for x in specifications_file.readlines()]

    # Parsing initial_specifications_lines into respective protobuf messages.
    messages = []
    STRING_TO_LATTICE_ELEMENT = {
        "bottom": proto.eesi_pb2.SignLatticeElement \
                  .SIGN_LATTICE_ELEMENT_BOTTOM,
        "<0": proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
        ">0": proto.eesi_pb2.SignLatticeElement \
              .SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
        "==0": proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_ZERO,
        "'==0": proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_ZERO,
        "<=0": proto.eesi_pb2.SignLatticeElement \
               .SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO,
        ">=0": proto.eesi_pb2.SignLatticeElement \
               .SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
        "!=0": proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_NOT_ZERO,
        "top": proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_TOP,
        "emptyset": proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_BOTTOM,
    }
    STRING_CONFIDENCE = {
        "bottom": (0, 0, 0, 0),
        "<0": (100, 0, 0, 0), 
        ">0": (0, 0, 100, 0),
        "==0": (0, 100, 0, 0),
        "'==0": (0, 100, 0, 0),
        "<=0": (100, 100, 0, 0),
        ">=0": (0, 100, 100, 0),
        "!=0": (100, 0, 100, 0),
        "top": (100, 100, 100, 0),
        "emptyset": (0, 0, 0, 100),
    }
    for line in specifications_lines:
        _, function, lattice_element = line.split()
        # The LLVM name will be the same as the source name
        # as the user-provided domain knowledge wouldn't
        # typically include the LLVM name.
        c_ltz, c_zero, c_gtz, c_empty = STRING_CONFIDENCE[lattice_element]
        function_message = proto.bitcode_pb2.Function(
            llvm_name=function,
            source_name=function,
        )
        specification_message = proto.eesi_pb2.Specification(
            function=function_message,
            lattice_element=STRING_TO_LATTICE_ELEMENT[lattice_element],
            confidence_zero=c_zero,
            confidence_less_than_zero=c_ltz,
            confidence_greater_than_zero=c_gtz,
            confidence_emptyset=c_empty
        )
        messages.append(specification_message)

    injected_response = proto.eesi_pb2.GetSpecificationsResponse(specifications=messages, violations=[])

    # Sending to eesi.rpc, which sends off request to bitcode service.
    cli.eesi.db.insert_specifications_response(
        database=database,
        request=request,
        response=injected_response,
    )

    cli.common.log.log_finished("InjectSpecifications")

def get_specifications_all(database, service_configuration_handler,
                           domain_knowledge_handler,
                           llm_name,
                           smart_success_code_zero,
                           overwrite):
    """Gets specifications for all bitcode files registered in MongoDB.

    Args:
        database: pymongo database object.
        service_configuration_handler: ServicesConfigurationHandler with
            services configured for: EESI, bitcode.
        domain_knowledge_handler: DomainKnowledgeHandler containing appropriate
            domain knowledge for a GetSpecifications task.
        llm_name: Name of the LLM to use, if any. 
        smart_success_code_zero: Whether to apply a heuristic to determine if 0
            is a success code in certain contexts, instead of every time.
        overwrite: Overwrites specification entries in MongoDB if an entry
            matches a registered bitcode ID.
    """

    # This is only for keeping track that if the database did update.
    changed = False
    partial_completion = False
    # Dictionary for keeping track of bitcode id to GetSpecificationsRequest.
    bitcode_id_requests = {}
    for uri in list(cli.bitcode.db.read_uris(database)):
        bitcode_id_handle = cli.bitcode.rpc.register_bitcode(
            service_configuration_handler.get_bitcode_stub(), uri)
        bitcode_id_handle.authority = \
            service_configuration_handler.bitcode_config.get_full_address()

        # Deleting current specifications in MongoDB if overwrite is True.
        if overwrite:
            cli.eesi.db.delete_specifications(
                database=database,
                bitcode_id=bitcode_id_handle.id,
            )
        else:
            # Checking if a GetSpecifications entry exists in database already.
            entry_exists = cli.db.db.collection_contains_id(
                database=database,
                id_type=ID_TYPE,
                unique_id=bitcode_id_handle.id,
                collection=COLLECTION_TYPE,
            )

            # If an entry already exists, do NOT get specifications for that
            # bitcode file.
            if entry_exists:
                log.info(f"Collection {COLLECTION_TYPE} already has an entry "
                         f"for bitcode id {bitcode_id_handle.id}.")
                partial_completion = True
                continue

        # Generating a request here instead of eesi.rpc because of the need to
        # match the applicable initial specifications with the individual
        # bitcode file.
        called_functions_response = \
            cli.bitcode.db.read_called_functions_response(
                database, bitcode_id_handle.id)
        defined_functions_response = \
            cli.bitcode.db.read_defined_functions_response(
                database, bitcode_id_handle.id)

        # The database must contain entries for both defined and called
        # functions. If no entry exists, attempt to run GetCalledFunctions
        # or GetDefinedFunctions before giving up.
        if not called_functions_response:
            called_functions_response = _setup_called_functions(
                database, service_configuration_handler, bitcode_id_handle.id,
                cli.common.uri.to_str(uri))

            if not called_functions_response:
                partial_completion = True
                continue

        if not defined_functions_response:
            defined_functions_response = _setup_defined_functions(
                database, service_configuration_handler, bitcode_id_handle.id,
                cli.common.uri.to_str(uri))

            if not defined_functions_response:
                partial_completion = True
                continue

        applicable_initial_specifications = \
            domain_knowledge_handler.get_applicable_specifications(
                called_functions_response, defined_functions_response)

        request = proto.eesi_pb2.GetSpecificationsRequest(
            bitcode_id=bitcode_id_handle,
            error_only_functions=domain_knowledge_handler.error_only,
            initial_specifications=applicable_initial_specifications,
            error_codes=domain_knowledge_handler.error_codes,
            success_codes=domain_knowledge_handler.success_codes,
            llm_name=llm_name,
            smart_success_code_zero=smart_success_code_zero,
        )
        bitcode_id_requests[bitcode_id_handle.id] = request
        changed = True # This means that all the prerequisites were met.

    # Sending to eesi.rpc and waiting for specifications responses
    cli.eesi.rpc.get_specifications(
        eesi_stub=service_configuration_handler.get_eesi_stub(),
        operations_stub=service_configuration_handler.get_operations_stub(
            "eesi"),
        bitcode_id_requests=bitcode_id_requests,
        max_tasks=service_configuration_handler.max_tasks,
        database=database,
    )

    cli.common.log.log_finished(
        "GetSpecificationsAll", changed, partial_completion)

def get_specifications_uri(database,
                           service_configuration_handler,
                           domain_knowledge_handler,
                           llm_name,
                           smart_success_code_zero, ctags_file,
                           bitcode_uri, overwrite,):
    """Get specifications for a single bitcode file registered with MongoDB.

    Args:
        database: pymongo database object.
        service_configuration_handler: ServicesConfigurationHandler with
            services configured for: EESI, bitcode
        domain_knowledge_handler: DomainKnowledgeHandler containing appropriate
            domain knowledge for a GetSpecifications task.
        llm_name: Bool to use the LLM for assistance in error specification
            inference.
        smart_success_code_zero: Whether to apply a heuristic to determine if 0
            is a success code in certain contexts, instead of every time.
        bitcode_uri: String representation of the bitcode file URI.
        overwrite: Overwrites specification entries in MongoDB if an entry
            matches a registered bitcode ID.
    """

    uri = cli.common.uri.parse(bitcode_uri)
    found_id = cli.bitcode.db.read_id_for_uri(database, uri)
    called_functions_response = None
    defined_functions_response = None
    if not found_id:
        found_id, called_functions_response, defined_functions_response = \
            _setup_bitcode(database, service_configuration_handler, uri)
        if not found_id:
            raise LookupError(f"Bitcode file: {uri.path} is not registering "
                              "with the bitcode service. Please check the "
                              "service log messages for any errors!")

    # Getting the bitcode handle.
    bitcode_id_handle = cli.bitcode.rpc.register_bitcode(
        service_configuration_handler.get_bitcode_stub(), uri)
    bitcode_id_handle.authority = \
        service_configuration_handler.bitcode_config.get_full_address()

    # Deleting current specifications in MongoDB
    # if they exist and overwrite is True.
    if overwrite:
        cli.eesi.db.delete_specifications(
            database=database,
            bitcode_id=bitcode_id_handle.id,
        )

    entry_exists = cli.db.db.collection_contains_id(
        database=database,
        id_type=ID_TYPE,
        unique_id=bitcode_id_handle.id,
        collection=COLLECTION_TYPE,
    )

    if entry_exists:
        log.info(f"Collection {COLLECTION_TYPE} already has an entry for "
                 f"bitcode id {bitcode_id_handle.id}.")
        cli.common.log.log_finished("GetSpecificationsUri", False)
        return

    # Getting called functions to evaluate which specifications
    # from domain knowledge are applicable to a given bitcode file. If
    # this bitcode file was not originally already registered. We should expect
    # to already have these responses
    if not called_functions_response:
        called_functions_response = \
            cli.bitcode.db.read_called_functions_response(
                database, bitcode_id_handle.id)
        if not called_functions_response:
            called_functions_response = _setup_called_functions(
                database, service_configuration_handler, bitcode_id_handle.id,
                bitcode_uri)
    if not defined_functions_response:
        defined_functions_response = \
            cli.bitcode.db.read_defined_functions_response(
                database, bitcode_id_handle.id)
        if not defined_functions_response:
            defined_functions_response = _setup_defined_functions(
                database, service_configuration_handler, bitcode_id_handle.id,
                bitcode_uri)

    # The database must contain entries for both defined and called functions.
    # If either of these conditions hold, then that means something has gone
    # wrong and you should check the logs.
    if not called_functions_response or not defined_functions_response:
        return

    # Using called functions and defined functions to get the
    # applicable initial specifications.
    applicable_initial_specifications = \
        domain_knowledge_handler.get_applicable_specifications(
            called_functions_response, defined_functions_response)

    # Generating a request here instead of eesi.rpc because of the need to
    # match the applicable initial specifications with the individual
    # bitcode file (this is primarily for get_specifications_all).
    request = proto.eesi_pb2.GetSpecificationsRequest(
        bitcode_id=bitcode_id_handle,
        error_only_functions=domain_knowledge_handler.error_only,
        initial_specifications=applicable_initial_specifications,
        error_codes=domain_knowledge_handler.error_codes,
        success_codes=domain_knowledge_handler.success_codes,
        ctags_file=ctags_file,
        llm_name=llm_name,
        smart_success_code_zero=smart_success_code_zero,
    )

    # Sending to eesi.rpc, which sends off request to bitcode service.
    cli.eesi.rpc.get_specifications(
        eesi_stub=service_configuration_handler.get_eesi_stub(),
        operations_stub=service_configuration_handler.get_operations_stub(
            "eesi"),
        bitcode_id_requests={bitcode_id_handle.id: request},
        max_tasks=service_configuration_handler.max_tasks,
        database=database
    )

    cli.common.log.log_finished("GetSpecificationsUri")


def list_specifications(database, bitcode_uri_filter, confidence_threshold, raw):
    """Prints out the specifications for the bitcode files in MongoDB.

    Args:
        database: Pymongo database object.
        bitcode_uri_filter: The bitcode URI to filter for, if set.
    """

    if not raw:
        print("---Specifications that appear in database---")
        print(colored(
            "Yellow indicates specification was part of domain knowledge!!!",
            "yellow"))
    id_file_dict = cli.bitcode.db.read_id_file_dict(database)
    for bitcode_id, bitcode_uri in id_file_dict.items():
        # Skip bitcode files not matching bitcode_uri_filter.
        if bitcode_uri_filter and bitcode_uri_filter != bitcode_uri:
            continue

        if not raw:
            # Printing headers for each bitcode entry.
            print("-"*30)
            print(colored(
                f"{'Bitcode ID (last 8 characters):':<40} "
                f"{'File name:':<75}",
                "red"))
            print(f"{bitcode_id[-8:]:40} {os.path.basename(bitcode_uri):<75}")
            print(colored(f"{'Function:':<50} {'Specification:':<30}", "green"))

        specifications_response = cli.eesi.db.read_specifications_response(
            database, bitcode_id)

        # If no specifications attached to entry, notify user.
        if not specifications_response:
            print("NONE FOUND")
            continue

        specifications_request = cli.eesi.db.read_specifications_request(
            database, bitcode_id)
        initial_specification_function_names = [
            spec.function.source_name
            for spec in specifications_request.initial_specifications]

        specifications = specifications_response
        for specification in specifications:
            specification_sign = LATTICE_ELEMENT_TO_STRING[
                cli.eesi.lattice_helper.get_lattice_element_from_confidence(
                    specification,
                    confidence_threshold)]
            if specification_sign == "bottom":
                continue

            function_name = specification.function.source_name
            if raw:
                if specification_sign == "==0":
                    specification_sign = "'==0"
                print(f"{function_name}: {function_name} {specification_sign}")
                continue
                

            if function_name in initial_specification_function_names:
                print(colored(f"{function_name:<50} {specification_sign:<30}",
                              "yellow"))
            else:
                print(f"{function_name:<50} {specification_sign:<30}")

def list_specifications_table(database, bitcode_uri_filter,
                              confidence_threshold, to_latex):
    """Prints out a table for the number of specifications for a database.

    Args:
        database: Pymongo database object.
        bitcode_uri_filter: The bitcode URI to filter for, if set.
        confidence_threshold: The minimum confidence for a component in the
            confidence/powerset lattice (==0, <0, >0) to be used as part of the
            final SignLatticeElement representative. These confidences are found
            in the Specification proto.
        to_latex: prints the rows of the specification counts in latex format.
            Note that this does not format the header, ID, or file name, as
            formatting these is useless for paper purposes.
    """

    id_file_dict = cli.bitcode.db.read_id_file_dict(database)
    # Printing off table header.
    print(colored(f"{'Bitcode ID (last 8 characters)':<40} {'File name:':<75} "
                  f"{'<0':<11} {'>0':<11} {'==0':<11} {'<=0':<11} {'>=0':<11} "
                  f"{'!=0':<11} {'top':<11} {'emptyset':<11} {'total':<12} "
                  f"{'increase %':<8}",
                  "green"))

    col_sep = ""
    row_end = ""
    if to_latex:
        col_sep = "&"
        row_end = "\\\\"

    for bitcode_id, bitcode_uri in sorted(id_file_dict.items(), key=lambda x: x[1]):
        # Skip bitcode files not matching bitcode_uri_filter.
        if bitcode_uri_filter and bitcode_uri_filter != bitcode_uri:
            continue

        specifications_response = cli.eesi.db.read_specifications_response(
            database, bitcode_id)

        # If no specifications attached to entry, notify user.
        if not specifications_response:
            print("NONE FOUND")
            continue


        inferred_specifications_count = defaultdict(int)

        specifications = specifications_response
        total = 0
        # Calculating total for the max confidence to see overall increase.
        total_baseline = 0
        for specification in specifications:
            specification_sign = LATTICE_ELEMENT_TO_STRING[
                cli.eesi.lattice_helper.get_lattice_element_from_confidence(
                    specification, confidence_threshold)]
            specification_sign_baseline = LATTICE_ELEMENT_TO_STRING[
                cli.eesi.lattice_helper.get_lattice_element_from_confidence(
                    specification, 100)]
            # Poorly adapted, but works for now.
            if specification_sign_baseline != "bottom":
                total_baseline += 1
            if specification_sign == "bottom":
                continue
                
            total += 1
            inferred_specifications_count[specification_sign] += 1

        increase = total - total_baseline
        if total_baseline:
            increase = (increase /  total_baseline) * 100.0
        print(f"{bitcode_id[-8:]:<40} {os.path.basename(bitcode_uri):<75} "
                f"{inferred_specifications_count['<0']:<7} {col_sep:<3} "
                f"{inferred_specifications_count['>0']:<7}  {col_sep:<3} "
                f"{inferred_specifications_count['==0']:<7}  {col_sep:<3} "
                f"{inferred_specifications_count['<=0']:<7} {col_sep:<3} "
                f"{inferred_specifications_count['>=0']:<7} {col_sep:<3} "
                f"{inferred_specifications_count['!=0']:<7} {col_sep:<3} "
                f"{inferred_specifications_count['top']:<7} {col_sep:<3} "
                f"{inferred_specifications_count['emptyset']:<7}  {col_sep:<3} "
                f"{total:<8} {col_sep:<3}"
              f"{increase:.2f} {row_end:<5}")

def specifications_table_to_csv(database, bitcode_uri_filter,
                                confidence_threshold, output_path):
    """Prints out specifications table to a csv file.

    Args:
        database: Pymongo database object.
        bitcode_uri_filter: bitcode_uri to filter for.
        confidence_threshold: The minimum confidence for a component in the
            confidence/powerset lattice (==0, <0, >0) to be used as part of the
            final SignLatticeElement representative. These confidences are found
            in the Specification proto.
        output_path: Absolute path to CSV file.
    """

    id_file_dict = cli.bitcode.db.read_id_file_dict(database)
    # Printing off table header.
    with open(output_path, "a") as output_file:
        output_file.write(
                f"{'Bitcode ID (last 8 characters)'}, {'File name:'}, "
                f"{'<0'}, {'>0'}, {'==0'}, {'<=0'}, {'>=0'}, {'!=0'}, "
                f"{'top'}, {'emptyset'}\n")

    for bitcode_id, bitcode_uri in id_file_dict.items():
        # Skip bitcode files not matching bitcode_uri_filter.
        if bitcode_uri_filter and bitcode_uri_filter != bitcode_uri:
            continue

        specifications_response = cli.eesi.db.read_specifications_response(
            database, bitcode_id)

        # If no specifications attached to entry, notify user.
        if not specifications_response:
            continue

        specifications_request = cli.eesi.db.read_specifications_request(
            database, bitcode_id)
        initial_specification_function_names = [
            spec.function.source_name
            for spec in specifications_request.initial_specifications]

        inferred_specifications_count = defaultdict(int)

        specifications = specifications_response
        for specification in specifications:
            function_name = specification.function.source_name
            specification_sign = LATTICE_ELEMENT_TO_STRING[
                cli.eesi.lattice_helper.get_lattice_element_from_confidence(
                    specification,
                    confidence_threshold)]

            if function_name not in initial_specification_function_names:
                inferred_specifications_count[specification_sign] += 1

        with open(output_path, "a") as output_file:
            output_file.write(
                f"{bitcode_id[-8:]}, {os.path.basename(bitcode_uri)}, "
                f"{inferred_specifications_count['<0']}, "
                f"{inferred_specifications_count['>0']}, "
                f"{inferred_specifications_count['==0']}, "
                f"{inferred_specifications_count['<=0']}, "
                f"{inferred_specifications_count['>=0']}, "
                f"{inferred_specifications_count['!=0']}, "
                f"{inferred_specifications_count['top']}, "
                f"{inferred_specifications_count['emptyset']}\n")

def list_specifications_diff(database_a, database_b, confidence_threshold,
                             bitcode_uri_filter):
    """Prints out the specifications difference between two databases.

    Args:
        database_a: Pymongo database object for first set of results.
        database_a: Pymongo database object for second set of results.
        confidence_threshold: The minimum confidence for a component in the
            confidence/powerset lattice (==0, <0, >0) to be used as part of the
            final SignLatticeElement representative. These confidences are found
            in the Specification proto.
        bitcode_uri_filter: The bitcode URI to filter for, if set.
    """

    id_file_dict_a = cli.bitcode.db.read_id_file_dict(database_a)
    id_file_dict_b = cli.bitcode.db.read_id_file_dict(database_b)
    # Merging the two dictionaries
    id_file_dict = id_file_dict_a.copy()
    id_file_dict.update(id_file_dict_b)

    for bitcode_id, bitcode_uri in id_file_dict.items():
        # Skip bitcode files not matching bitcode_uri_filter.
        if bitcode_uri_filter and bitcode_uri_filter != bitcode_uri:
            continue

        print("-"*30)
        # If bitcode file not registered in both databases, skip it.
        if bitcode_id not in id_file_dict_b or bitcode_id not in id_file_dict_a:
            print(f"Bitcode file: {os.path.basename(bitcode_uri)} with "
                  f"ID: {bitcode_id} not registered in both databases.")
            continue

        specifications_response_a = cli.eesi.db.read_specifications_response(
            database_a, bitcode_id)
        specifications_response_b = cli.eesi.db.read_specifications_response(
            database_b, bitcode_id)

        # If at least one database does not have specification entries
        # for the bitcode file, skip it.
        if not specifications_response_a or not specifications_response_b:
            print(f"Bitcode file: {os.path.basename(bitcode_uri)} with ID: "
                  f"{bitcode_id} does not have entries for specifications in "
                  "both databases. Skipping diff!")
            continue

        # Printing headers for each bitcode entry.
        print(colored(f"{'Bitcode ID (last 8 characters):':<40} "
                      f"{'File name:':<75}",
                      "red"))
        print(f"{bitcode_id[-8:]:40} {os.path.basename(bitcode_uri):<75}")
        print(colored(f"{'Function:':<50} {'Specification:':<30}", "green"))

        # Generating two specifications dict for each database.
        specifications_dict_a = {
            specification.function.source_name:
                LATTICE_ELEMENT_TO_STRING[
                    cli.eesi.lattice_helper.get_lattice_element_from_confidence(
                        specification,
                        confidence_threshold)]
            for specification in specifications_response_a}
        specifications_dict_b = {
            specification.function.source_name:
                LATTICE_ELEMENT_TO_STRING[
                    cli.eesi.lattice_helper.get_lattice_element_from_confidence(
                        specification,
                        confidence_threshold)]
            for specification in specifications_response_b}

        # Printing specifications in A, but not in B.
        for function_name, lattice_element in specifications_dict_a.items():
            if (function_name not in specifications_dict_b or
                    specifications_dict_b[function_name] != lattice_element):
                print(colored(f"{function_name:<50} {lattice_element:<30}",
                              "blue"))
        # Printing specifications in B, but not in A.
        for function_name, lattice_element in specifications_dict_b.items():
            if (function_name not in specifications_dict_a or
                    specifications_dict_a[function_name] != lattice_element):
                print(colored(f"{function_name:<50} {lattice_element:<30}",
                              "red"))


# TODO: The below was implemented extremely piecemeal at the time.
 # Incredibly poorly-written and should be redone.
def list_statistics_database(bitcode_uri, ground_truth_file, database, t):
    """ Lists statistics from specifications inferred against ground-truth."""

    ground_truth = dict()
    ground_truth_non_bot_count = 0
    third_party_gt = 0
    third_party = set()
    with open(ground_truth_file) as f:
        lines = f.readlines()
        for l in lines:
            l = l.split()
            if len(l) < 1 or len(l) > 3:
                print("Ground-truth file has bad formatting!")
                print("Rows must be between 1 and 3 columns!")
                return
            fname = l[0]
            if len(l) < 2 or l[1] == "X":
                ground_truth[fname] = "bottom"
                continue
            if len(l) == 3:
                if l[2] != "X":
                    print("Ground-truth file has bad formatting!")
                    print("Third column is not an X!")
                    return
                third_party.add(fname)
            #if l[1] == "emptyset":
            #    continue
            spec = l[1]
            ground_truth[fname] = spec
            ground_truth_non_bot_count += 1
            if len(l) == 3:
                third_party_gt += 1

    if not ground_truth:
        print("No ground truth!")
        return

    bitcode_id = cli.bitcode.db.read_id_for_uri(
        database, cli.common.uri.parse(bitcode_uri))
    if not bitcode_id:
        print("Bitcode ID not in database!")
        return
    get_specifications_response = cli.eesi.db.read_specifications_response(
        database, bitcode_id)
    if not get_specifications_response:
        print("No GetSpecificationsResponse in database!")
        return
    get_specifications_request = cli.eesi.db.read_specifications_request(
        database, bitcode_id)
    if not get_specifications_request:
        print("No GetSpecificationsRequest in database!")
        return
    init_specs = set()
    tp_init_specs = set()
    empty_init_specs = set()
    for init_spec in get_specifications_request.initial_specifications:
        # We only care about newly inferred third-party functions, remove the
        # initial specifications from the set.
        if init_spec.function.source_name in third_party:
            third_party.remove(init_spec.function.source_name)
            tp_init_specs.add(init_spec.function.source_name)
        if LATTICE_ELEMENT_TO_STRING[init_spec.lattice_element] == "bottom":
            empty_init_specs.add(init_spec.function.source_name)
        if LATTICE_ELEMENT_TO_STRING[init_spec.lattice_element] == "emptyset":
            empty_init_specs.add(init_spec.function.source_name)
        if init_spec.function.source_name not in ground_truth:
            continue
        init_specs.add(init_spec.function.source_name)

    tp = 0
    fp = 0
    inferred = 0
    unique_inferred = 0
    unique_tp = 0
    unique_fp = 0
    ground_truth_non_bot_count = ground_truth_non_bot_count - len(init_specs)
    total = ground_truth_non_bot_count 
    total_non_init = ground_truth_non_bot_count 
    total_third_party = len(third_party)
    third_party_tp = 0
    third_party_fp = 0
    for specification in get_specifications_response:
        fname = specification.function.source_name
        le = LATTICE_ELEMENT_TO_STRING[
            cli.eesi.lattice_helper.get_lattice_element_from_confidence(
                specification, t)]
        # Calaculting the base lattice element from the analysis (no expansion).
        base_le = LATTICE_ELEMENT_TO_STRING[
            cli.eesi.lattice_helper.get_lattice_element_from_confidence(
                specification, 100)]
        unique = le != base_le
        if fname not in ground_truth or fname in init_specs:
            continue

        # FIXME(patrickjchap): I don't know what I was thinking before, but
        # the logic below here is terrible.
        inferred += 1
        if unique:
            unique_inferred += 1
        if ground_truth[fname] == le:
            if le == "bottom":
                inferred -= 1
                if unique:
                    unique_inferred -= 1
                continue
            tp += 1
            if unique:
                unique_tp += 1
            if fname in third_party:
                third_party_tp += 1
        elif ground_truth[fname] != le:
            if le == "bottom":
                inferred -= 1
                if unique:
                    unique_inferred -= 1
                continue
            if ground_truth[fname] == "bottom":
                continue
            print("="*80)
            print(f"false positive {fname}: {le}")
            print(f"Inferred via LLM: {specification.inferred_with_llm}")
            print(f"Sources ==0: {specification.sources_of_inference_zero}")
            print(f"Sources <0: {specification.sources_of_inference_less_than_zero}")
            print(f"Sources >0: {specification.sources_of_inference_greater_than_zero}")
            print(f"Sources EMPTYSET: {specification.sources_of_inference_emptyset}")
            print("="*80)
            fp += 1
            if unique:
                unique_fp += 1
            if fname in third_party:
                third_party_fp += 1
    fn = total_non_init - (tp + fp)
    third_party_fn = (total_third_party - (third_party_fp + third_party_tp))
    precision = 0.0
    if (tp + fp) > 0:
        precision = 100 * (tp / (tp + fp))
    third_party_precision = 0.0
    if third_party_tp or third_party_fp:
        third_party_precision = 100 * (third_party_tp / (third_party_tp + third_party_fp))
    third_party_inferred = third_party_tp + third_party_fp
    third_party_coverage = 0.0
    if total_third_party:
        third_party_coverage = 100 * (third_party_inferred / total_third_party)
    delta_precision = 0.0
    if unique_inferred != 0:
        delta_precision = 100 * (unique_tp / (unique_tp + unique_fp))
    coverage = 100 * ( (tp + fp) / total_non_init)
    f1 = 0.0
    recall = 0.0
    if tp + fn > 0:
        print(f"FN: {fn}")
        recall = 100.0 * (tp / (fn + tp))
    if precision + recall> 0:
        f1 = (2 * (precision * recall) / (precision + recall)) / 100
    third_party_recall = 0.0
    if third_party_tp + third_party_fn > 0:
        third_party_recall = 100.0 * (third_party_tp / (third_party_fn + third_party_tp))
    print(10*"=", f"Statistics for confidence threshold: {t}", 10*"=")
    print(f"Total Functions: {total}")
    print(f"True positives: {tp}; False positives: {fp}")
    print(f"Total Functions Ground Truth(not unknown): {ground_truth_non_bot_count}")
    print(f"Total 3rd Functions Ground Truth(not unknown): {third_party_gt}")
    print(f"Total Non-Init. Functions: {total_non_init}")
    print(f"Init. Functions: {len(init_specs)}")
    print(f"3rd Init. Functions: {len(tp_init_specs)}")
    print(f"Init. Emptyset Functions: {len(empty_init_specs)}")
    print(f"Total Newly Inferred: {inferred}")
    print(f"Total Total Inferred: {inferred + len(init_specs)}")
    print(f"Precision: {precision:.2f}%")
    print(f"Recall: {recall:.2f}%")
    print(f"Coverage: {coverage:.2f}%")
    print(f"F1: {f1}")
    print(f"Delta Precision over baseline: {delta_precision:.2f}%")
    print(f"3rd party Total Eligible: {len(third_party)}")
    print(f"3rd party Total Newly Inferred: {third_party_inferred}")
    print(f"3rd party Precision: {third_party_precision:.2f}%")
    print(f"3rd party Recall: {third_party_recall:.2f}%")
    print(f"3rd party Coverage: {third_party_coverage:.2f}%")

def list_statistics_file(ground_truth_file, init_specs_path, specs_path, t):
    """ Lists statistics from specifications inferred against ground-truth."""

    ground_truth = dict()
    ground_truth_non_bot_count = 0
    third_party_gt = 0
    third_party = set()
    with open(ground_truth_file) as f:
        lines = f.readlines()
        for l in lines:
            l = l.split()
            if len(l) < 1 or len(l) > 3:
                print("Ground-truth file has bad formatting!")
                print("Rows must be between 1 and 3 columns!")
                return
            fname = l[0]
            if len(l) < 2 or l[1] == "X":
                ground_truth[fname] = "bottom"
                continue
            if len(l) == 3:
                if l[2] != "X":
                    print("Ground-truth file has bad formatting!")
                    print("Third column is not an X!")
                    return
                third_party.add(fname)
            if l[1] == "emptyset":
                continue
            spec = l[1]
            ground_truth[fname] = spec
            ground_truth_non_bot_count += 1
            if len(l) == 3:
                third_party_gt += 1

    specifications = dict()
    with open(specs_path) as f:
        lines = f.readlines()
        for l in lines:
            l = l.split()
            if len(l) < 1 or len(l) > 3:
                print("Specifications file has bad formatting!")
                return
            if len(l) == 3:
                specifications[l[1]] = l[2]
                continue
            if len(l) == 2:
                specifications[l[0]] = l[1]
                continue
            if len(l) == 1:
                specifications[l[0]] = "bottom"
                continue
    if not specifications:
        print("No specifications!")
        return

    init_specs = set()
    empty_init_specs = set() 
    with open(init_specs_path) as f:
        lines = f.readlines()
        for l in lines:
            l = l.split()
            if len(l) < 2:
                print("Specifications file has bad formatting!")
                return
            if l[0] not in ground_truth:
                continue
            if l[1] == "bottom":
                empty_init_specs.add(l[0])
                continue
            init_specs.add(l[0])
    if not init_specs:
        print("No initial specifications!")

    tp = 0
    fp = 0
    inferred = 0
    total = len(ground_truth)
    total_non_init = len(ground_truth) - len(init_specs)
    for fname, le in specifications.items():
        if fname not in ground_truth or fname in init_specs:
            continue

        inferred += 1
        if ground_truth[fname] == le:
            if le == "bottom":
                inferred -= 1
                continue
            tp += 1
        elif ground_truth[fname] != le:
            if le == "bottom":
                inferred -= 1
                continue
            if ground_truth[fname] == "bottom":
                continue
            fp += 1
    precision = 100 * (tp / (tp + fp))
    recall = 100 * (inferred / total_non_init)
    f1 = (2 * (precision * recall) / (precision + recall)) / 100
    print(10*"=", f"Statistics for confidence threshold: {t}", 10*"=")
    print(f"Total Functions: {total}")
    print(f"Total Non-Init. Functions: {total_non_init}")
    print(f"Init. Functions: {len(init_specs)}")
    print(f"Init. Emptyset Functions: {len(empty_init_specs)}")
    print(f"Total Newly Inferred: {inferred}")
    print(f"Total Total Inferred: {inferred + len(init_specs)}")
    print(f"Precision: {precision:.2f}%")
    print(f"Recall: {recall:.2f}%")
    print(f"F1: {f1}")
