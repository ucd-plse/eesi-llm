"""Commands for various tasks associated with the bitcode service.

Commands associated with bitcode service tasks. These tasks include bitcode
registration, getting called functions for a bitcode file, and getting defined
functions for a bitcode file. Information about the tasks are stored in a
MongoDB.
"""

import os
import sys
sys.path = [path for path in sys.path if "com_" not in path]

import glog as log
from termcolor import colored

import cli.bitcode.db
import cli.bitcode.rpc
import cli.common.uri
import cli.db.db

ID_TYPE = "request.bitcodeId.id"

def register_bitcode(database, service_configuration_handler, uri, overwrite):
    """Registers bitcode with service and inserts URI and ID in DB.

    Calls register_bitcode from bitcode.rpc and inserts the resulting bitcode
    ID and URI into the provided DB.

    Args:
        database: Pymongo database object.
        service_configuration_handler: ServiceConfigurationHandler configured
            for the bitcode service.
        uri: The string URI of the bitcode file to register
        overwrite: If true, overwrites database 'filenames' entries.
    """

    uri = cli.common.uri.parse(uri)
    if overwrite:
        cli.bitcode.db.delete_filenames_entry(database, uri)

    if cli.bitcode.db.read_id_for_uri(database, uri):
        log.error("Bitcode file {} already registered with database. "
                  "Use --overwrite if you wish to overwrite entries!".format(
                      cli.common.uri.to_str(uri)))
        return

    bitcode_id_handle = cli.bitcode.rpc.register_bitcode(
        service_configuration_handler.get_bitcode_stub(), uri)

    # Inserting into DB.
    cli.bitcode.db.insert_bitcode(
        database=database,
        bitcode_id=bitcode_id_handle,
        uri=uri,
    )

    log.info("RegisterBitcode command has finished! "
             "Populated entries in MongoDB.")

def re_register_bitcode(database, service_configuration_handler, uri,
                        overwrite):
    """Wrapper with logging around RegisterBitcode for other commands. """
    log.warning(f"{cli.common.uri.to_str(uri)} URI not found in database!"
                " Registering bitcode file first!")
    register_bitcode(database, service_configuration_handler,
                     cli.common.uri.to_str(uri), overwrite)
    found_id = cli.bitcode.db.read_id_for_uri(database, uri)
    if not found_id:
        log.error(f"{cli.common.uri.to_str(uri)} URI not found in database even"
                  " after attempting to register the bitcode file. Please check"
                  " the bitcode service for any potential errors.")
        return None

    # Everything is okay
    return found_id

def register_local_dataset(database, service_configuration_handler, uri,
                           overwrite):
    """Registers all local bitcode files in a given directory URI.

    Get all local bitcode files from a directory's URI and then registers
    each bitcode file individually with bitcode.rpc and inserts
    each bitcode file's URI and ID into the DB.

    Args:
        database: Pymongo database object.
        service_configuration_handler: ServiceConfigurationHandler configured
            for the bitcode service.
        uri: The string base URI where bitcode files are stored locally.
        overwrite: If true, overwrites database 'filenames' entries.
    """

    uri = cli.common.uri.parse(uri)
    uri_path = uri.path
    for root, dirs, files in os.walk(uri_path):
        del dirs # Not used.
        for f in files:
            f_abspath = os.path.join(root, f)

            # We assume bitcode files only use .bc or .ll extensions
            if f.endswith(".bc") or f.endswith(".ll"):
                # Creating a uri for the individual bitcode file
                str_uri = "file://{}".format(f_abspath)
                bitcode_uri = cli.common.uri.parse(str_uri)
                # Delete entry if overwrite is true.
                if overwrite:
                    cli.bitcode.db.delete_filenames_entry(database, bitcode_uri)

                # Skip registration if already registered in database.
                if cli.bitcode.db.read_id_for_uri(database, bitcode_uri):
                    log.error(
                        f"Bitcode file {cli.common.uri.to_str(bitcode_uri)}"
                        " already registered with database. Use --overwrite if"
                        " you wish to overwrite entries!")
                    continue

                # Checking if bitcode with matching id already
                # registered in the database
                if cli.bitcode.db.read_id_for_uri(database, bitcode_uri):
                    log.info(f"{str_uri} already registered, Skipping.")

                # Getting bitcode id from bitcode.rpc
                bitcode_id = cli.bitcode.rpc.register_bitcode(
                    service_configuration_handler.get_bitcode_stub(),
                    bitcode_uri)
                # Inserting bitcode information into database
                cli.bitcode.db.insert_bitcode(
                    database=database,
                    bitcode_id=bitcode_id,
                    uri=bitcode_uri,)

                log.info("Registered bitcode file {} from dataset.".format(
                    f_abspath))

    log.info("RegisterLocalDataset command has finished! "
             "Populated entries in MongoDB.")

def get_called_functions_all(database, service_configuration_handler,
                             overwrite):
    """Gets called functions for bitcode in DB and stores called functions.

    Gets called functions for all bitcode files that are stored in DB and then
    stores all request and response information in DB that pertain to the
    called functions.

    Args:
        database: Pymongo database object.
        service_configuration_handler: ServiceConfigurationHandler configured
            for at least the bitcode service.
        overwrite: Overwrites called functions per entry if set True.
    """

    # Delete all previous called function entries if they exist
    if overwrite:
        cli.bitcode.db.delete_called_functions_all(database)

    # Lookup bitcode ID for URI in database.
    # Create stubs for connecting to servers.
    id_type = "request.bitcodeId.id"
    collection_type = "CalledFunctionsResponse"

    uris = cli.bitcode.db.read_uris(database)

    bitcode_id_handles = []
    for uri in uris:
        bitcode_id_handle = cli.bitcode.rpc.register_bitcode(
            service_configuration_handler.get_bitcode_stub(), uri)
        bitcode_id_handle.authority = \
            service_configuration_handler.bitcode_config.get_full_address()

        # Lookup if entry with bitcode id already exists in
        # CalledFunctionsResponse collection
        entry_exists = cli.db.db.collection_contains_id(
            database=database,
            id_type=id_type,
            unique_id=bitcode_id_handle.id,
            collection=collection_type,
        )

        # If entry exists for bitcode id, do nothing with it
        if entry_exists:
            log.info(f"Collection {collection_type} already has an entry for"
                     f" bitcode id {bitcode_id_handle.id}.")

            continue

        bitcode_id_handles.append(bitcode_id_handle)

    # Insert response from GetCalledFunctions into database.
    cli.bitcode.rpc.get_called_functions(
        service_configuration_handler.get_bitcode_stub(),
        service_configuration_handler.get_operations_stub("bitcode"),
        bitcode_id_handles,
        database,
        service_configuration_handler.max_tasks,
    )

    log.info("GetCalledFunctionAll command has finished! "
             "Populated entries in MongoDB.")

def get_called_functions_uri(database, service_configuration_handler,
                             uri, overwrite):
    """Gets called functions for a single bitcode file and store in DB.

    Args:
        database: Pymongo database object.
        service_configuration_handler: ServiceConfigurationHandler configured
            for at least the bitcode service.
        uri: String representation of the uri to a single bitcode file
        overwrite: Overwrites called functions if set True.
    """

    # Keeping string copy for logging
    uri = cli.common.uri.parse(uri)
    collection_type = "CalledFunctionsResponse"

    found_id = cli.bitcode.db.read_id_for_uri(database, uri)
    if not found_id:
        found_id = re_register_bitcode(database, service_configuration_handler,
                                       uri, overwrite)
        if not found_id:
            raise LookupError(f"Bitcode file: {uri.path} is not registering "
                              "with the bitcode service. Please check the "
                              "service log messages for any errors!")

    bitcode_id_handle = cli.bitcode.rpc.register_bitcode(
        service_configuration_handler.get_bitcode_stub(), uri)
    bitcode_id_handle.authority = \
        service_configuration_handler.bitcode_config.get_full_address()

    # If overwrite is true, remove single entry.
    if overwrite:
        cli.bitcode.db.delete_called_functions(database, bitcode_id_handle)

    # Looking if entry with same bitcode id exists in
    # CalledFunctionsResponse collection
    entry_exists = cli.db.db.collection_contains_id(
        database=database,
        unique_id=bitcode_id_handle.id,
        collection=collection_type,
        id_type="request.bitcodeId.id",
    )

    # If entry already exists, do nothing with that bitcode file.
    if entry_exists:
        log.info(f"Collection {collection_type} already has an entry for"
                 f" bitcode ID {bitcode_id_handle.id}")
        return

    # Calling get_called_functions for a single bitcode file.
    cli.bitcode.rpc.get_called_functions(
        service_configuration_handler.get_bitcode_stub(),
        service_configuration_handler.get_operations_stub("bitcode"),
        [bitcode_id_handle],
        database,
        service_configuration_handler.max_tasks,
    )

    log.info("GetCalledFunctionsUri command has finished! "
             "Populated entries in MongoDB.")

def get_local_called_functions_uri(database, service_configuration_handler,
                                   uri, overwrite):
    """Gets local called functions for a single bitcode file and store in DB.

    Args:
        database: Pymongo database object.
        service_configuration_handler: ServiceConfigurationHandler configured
            for at least the bitcode service.
        uri: String representation of the uri to a single bitcode file.
        overwrite: Overwrites local called functions if set True.
    """

    # Keeping string copy for logging
    uri = cli.common.uri.parse(uri)
    collection_type = "LocalCalledFunctionsResponse"

    found_id = cli.bitcode.db.read_id_for_uri(database, uri)
    if not found_id:
        found_id = re_register_bitcode(database, service_configuration_handler,
                                       uri, overwrite)
        if not found_id:
            raise LookupError(f"Bitcode file: {uri.path} is not registering "
                              "with the bitcode service. Please check the "
                              "service log messages for any errors!")

    bitcode_id_handle = cli.bitcode.rpc.register_bitcode(
        service_configuration_handler.get_bitcode_stub(), uri)
    bitcode_id_handle.authority = \
        service_configuration_handler.bitcode_config.get_full_address()

    # If overwrite is true, remove single entry.
    if overwrite:
        cli.bitcode.db.delete_local_called_functions(database,
                                                     bitcode_id_handle)

    # Looking if entry with same bitcode id exists in
    # LocalCalledFunctionsResponse collection
    entry_exists = cli.db.db.collection_contains_id(
        database=database,
        unique_id=bitcode_id_handle.id,
        collection=collection_type,
        id_type="request.bitcodeId.id",
    )

    # If entry already exists, do nothing with that bitcode file.
    if entry_exists:
        log.info(f"Collection {collection_type} already has an entry for"
                 f" bitcode ID {bitcode_id_handle.id}.")
        return

    # Calling GetLocalCalledFunctions for a single bitcode file.
    cli.bitcode.rpc.get_local_called_functions(
        service_configuration_handler.get_bitcode_stub(),
        service_configuration_handler.get_operations_stub("bitcode"),
        [bitcode_id_handle],
        database,
        service_configuration_handler.max_tasks,
    )

    log.info("GetLocalCalledFunctionsUri command has finished! "
             "Populated entries in MongoDB.")

def get_local_called_functions_all(database, service_configuration_handler,
                                   overwrite):
    """Gets local called functions for all bitcode in DB and stores in DB.

    Args:
        database: Pymongo database object.
        service_configuration_handler: ServiceConfigurationHandler configured
            for at least the bitcode service.
        overwrite: Overwrites local called functions per entry if set True.
    """

    # Delete all previous local called function entries if they exist
    if overwrite:
        cli.bitcode.db.delete_local_called_functions_all(database)

    # Lookup bitcode ID for URI in database.
    # Create stubs for connecting to servers.
    id_type = "request.bitcodeId.id"
    collection_type = "LocalCalledFunctionsResponse"

    uris = cli.bitcode.db.read_uris(database)

    bitcode_id_handles = []
    for uri in uris:
        bitcode_id_handle = cli.bitcode.rpc.register_bitcode(
            service_configuration_handler.get_bitcode_stub(), uri)
        bitcode_id_handle.authority = \
            service_configuration_handler.bitcode_config.get_full_address()

        # Lookup if entry with bitcode id already exists in
        # LocalCalledFunctionsResponse collection
        entry_exists = cli.db.db.collection_contains_id(
            database=database,
            id_type=id_type,
            unique_id=bitcode_id_handle.id,
            collection=collection_type,
        )

        # If entry exists for bitcode id, do nothing with it
        if entry_exists:
            log.info(f"Collection {collection_type} already has an entry for"
                     f" bitcode id {bitcode_id_handle.id}.")
            continue

        bitcode_id_handles.append(bitcode_id_handle)

    # Insert response from GetLocalCalledFunctions into database.
    cli.bitcode.rpc.get_local_called_functions(
        service_configuration_handler.get_bitcode_stub(),
        service_configuration_handler.get_operations_stub("bitcode"),
        bitcode_id_handles,
        database,
        service_configuration_handler.max_tasks,
    )

    log.info("GetLocalCalledFunctionAll command has finished! "
             "Populated entries in MongoDB.")

def get_file_called_functions_uri(database, service_configuration_handler,
                                  uri, overwrite):
    """Gets file called functions for a single bitcode file and store in DB.

    Args:
        database: Pymongo database object.
        service_configuration_handler: ServiceConfigurationHandler configured
            for at least the bitcode service.
        uri: String representation of the uri to a single bitcode file.
        overwrite: Overwrites file called functions if set True.
    """

    # Keeping string copy for logging
    uri = cli.common.uri.parse(uri)
    collection_type = "FileCalledFunctionsResponse"

    found_id = cli.bitcode.db.read_id_for_uri(database, uri)
    if not found_id:
        found_id = re_register_bitcode(database, service_configuration_handler,
                                       uri, overwrite)
        if not found_id:
            raise LookupError(f"Bitcode file: {uri.path} is not registering "
                              "with the bitcode service. Please check the "
                              "service log messages for any errors!")

    bitcode_id_handle = cli.bitcode.rpc.register_bitcode(
        service_configuration_handler.get_bitcode_stub(), uri)
    bitcode_id_handle.authority = \
        service_configuration_handler.bitcode_config.get_full_address()

    # If overwrite is true, remove single entry.
    if overwrite:
        cli.bitcode.db.delete_file_called_functions(database,
                                                    bitcode_id_handle)

    # Looking if entry with same bitcode id exists in
    # FileCalledFunctionsResponse collection
    entry_exists = cli.db.db.collection_contains_id(
        database=database,
        unique_id=bitcode_id_handle.id,
        collection=collection_type,
        id_type="request.bitcodeId.id",
    )

    # If entry already exists, do nothing with that bitcode file.
    if entry_exists:
        log.info(f"Collection {collection_type} already has an entry for"
                 f" bitcode ID {bitcode_id_handle.id}.")
        return

    # Calling GetLocalCalledFunctions for a single bitcode file.
    cli.bitcode.rpc.get_file_called_functions(
        service_configuration_handler.get_bitcode_stub(),
        service_configuration_handler.get_operations_stub("bitcode"),
        [bitcode_id_handle],
        database,
        service_configuration_handler.max_tasks,
    )

    log.info("GetFileCalledFunctionsUri command has finished! "
             "Populated entries in MongoDB.")

def get_file_called_functions_all(database, service_configuration_handler,
                                  overwrite):
    """Gets file called functions for all bitcode in DB and stores in DB.

    Args:
        database: Pymongo database object.
        service_configuration_handler: ServiceConfigurationHandler configured
            for at least the bitcode service.
        overwrite: Overwrites file called functions per entry if set True.
    """

    # Delete all previous local called function entries if they exist
    if overwrite:
        cli.bitcode.db.delete_file_called_functions_all(database)

    # Lookup bitcode ID for URI in database.
    # Create stubs for connecting to servers.
    id_type = "request.bitcodeId.id"
    collection_type = "FileCalledFunctionsResponse"

    uris = cli.bitcode.db.read_uris(database)

    bitcode_id_handles = []
    for uri in uris:
        bitcode_id_handle = cli.bitcode.rpc.register_bitcode(
            service_configuration_handler.get_bitcode_stub(), uri)
        bitcode_id_handle.authority = \
            service_configuration_handler.bitcode_config.get_full_address()

        # Lookup if entry with bitcode id already exists in
        # FileCalledFunctionsResponse collection
        entry_exists = cli.db.db.collection_contains_id(
            database=database,
            id_type=id_type,
            unique_id=bitcode_id_handle.id,
            collection=collection_type,
        )

        # If entry exists for bitcode id, do nothing with it
        if entry_exists:
            log.info("Collection {} already has an entry for bitcode id {}"
                     .format(collection_type, bitcode_id_handle.id))

            continue

        bitcode_id_handles.append(bitcode_id_handle)

    # Insert response from GetFileCalledFunctions into database.
    cli.bitcode.rpc.get_file_called_functions(
        service_configuration_handler.get_bitcode_stub(),
        service_configuration_handler.get_operations_stub("bitcode"),
        bitcode_id_handles,
        database,
        service_configuration_handler.max_tasks,
    )

    log.info("GetFileCalledFunctionAll command has finished! "
             "Populated entries in MongoDB.")

def get_defined_functions_all(database, service_configuration_handler,
                              overwrite):
    """Gets defined functions for bitcode in DB and stores defined functions.

    Gets defined functions for all bitcode files that are stored in DB and then
    stores all request and response information in DB that pertain to the
    defined functions.

    Args:
        database: Pymongo database object.
        service_configuration_handler: ServiceConfigurationHandler configured
            for at least the bitcode service.
        overwrite: Overwrites defined functions per entry if set True.
    """

    # Delete all previous defined functions entries if overwrite
    if overwrite:
        cli.bitcode.db.delete_defined_functions_all(database)

    # Lookup bitcode ID for URI in database.
    # Create stubs for connecting to servers.
    collection_type = "DefinedFunctionsResponse"
    uris = cli.bitcode.db.read_uris(database)
    bitcode_id_handles = []
    for uri in uris:
        bitcode_id_handle = cli.bitcode.rpc.register_bitcode(
            service_configuration_handler.get_bitcode_stub(), uri)
        bitcode_id_handle.authority = \
            service_configuration_handler.bitcode_config.get_full_address()

        # Look if entry already exists for the given bitcode id
        # in the DefinedFunctionsResponse collection
        entry_exists = cli.db.db.collection_contains_id(
            database=database,
            unique_id=bitcode_id_handle.id,
            collection=collection_type,
            id_type="request.bitcodeId.id",
        )

        # If entry already exists for bitcode id, do nothing with it
        if entry_exists:
            log.info(f"Collection {collection_type} already has an entry for"
                     f" bitcode id {bitcode_id_handle.id}.")
            continue

        bitcode_id_handles.append(bitcode_id_handle)

    # Insert response from GetCalledFunctions into database.
    cli.bitcode.rpc.get_defined_functions(
        service_configuration_handler.get_bitcode_stub(),
        service_configuration_handler.get_operations_stub("bitcode"),
        bitcode_id_handles,
        database,
        service_configuration_handler.max_tasks,
    )

    log.info("GetDefinedFunctionsAll command has finished! "
             "Populated entries in MongoDB.")

def get_defined_functions_uri(database, service_configuration_handler,
                              uri, overwrite):
    """ Get defined functions for a single bitcode file from a uri and then
        store the defined functions in the datbase

    Args:
        database: Pymongo database object.
        service_configuration_handler: ServiceConfigurationHandler configured
            for the bitcode service.
        uri: String representation of the uri to a single bitcode file.
        overwrite: Overwrites defined functions if set True.
    """

    collection_type = "DefinedFunctionsResponse"
    uri = cli.common.uri.parse(uri)

    found_id = cli.bitcode.db.read_id_for_uri(database, uri)
    if not found_id:
        found_id = re_register_bitcode(database, service_configuration_handler,
                                       uri, overwrite)
        if not found_id:
            raise LookupError(f"Bitcode file: {uri.path} is not registering "
                              "with the bitcode service. Please check the "
                              "service log messages for any errors!")

    bitcode_id_handle = cli.bitcode.rpc.register_bitcode(
        service_configuration_handler.get_bitcode_stub(), uri)
    bitcode_id_handle.authority = \
        service_configuration_handler.bitcode_config.get_full_address()

    # If overwrite, remove single entry
    if overwrite:
        cli.bitcode.db.delete_defined_functions(database, bitcode_id_handle)

    # Look up if entry already exists for a bitcode id
    # in DefinedFunctionsResponse collection
    entry_exists = cli.db.db.collection_contains_id(
        database=database,
        unique_id=bitcode_id_handle.id,
        collection=collection_type,
        id_type="request.bitcodeId.id",
    )

    # If entry exists, do nothing with bitcode id
    if entry_exists:
        log.info(f"Collection {collection_type} already has an entry for"
                 f" bitcode id {bitcode_id_handle.id}")
        return

    cli.bitcode.rpc.get_defined_functions(
        service_configuration_handler.get_bitcode_stub(),
        service_configuration_handler.get_operations_stub("bitcode"),
        [bitcode_id_handle],
        database,
        service_configuration_handler.max_tasks
    )

    log.info("GetDefinedFunctionsUri command has finished! "
             "Populated entries in MongoDB.")

def list_registered_bitcode(database):
    """Prints out the bitcode files that are registered with MongoDB."""

    print("---Registered bitcode files in database---")
    id_file_dict = cli.bitcode.db.read_id_file_dict(database)
    print(colored(
        f"{'Bitcode ID (last 8 characters):':<40} {'File name:':<75}",
        "red"))
    for bitcode_id, bitcode_uri in id_file_dict.items():
        print(f"{bitcode_id[-8:]:<40} {os.path.basename(bitcode_uri):<75}")

def list_called_functions(database):
    """Prints out the called functions for the bitcode files in MongoDB."""

    print("---Called functions that appear in database---")
    id_file_dict = cli.bitcode.db.read_id_file_dict(database)
    id_called_functions_dict = cli.bitcode.db.read_id_called_functions_dict(
        database)

    if not id_called_functions_dict:
        log.error("No called functions found for any bitcode file!")
        log.error("Make sure that GetCalledFunctionsAll "
                  "or GetCalledFunctionsUri has been ran!")
        sys.exit(1)

    for bitcode_id, bitcode_uri in id_file_dict.items():
        # Printing out header entries for each bitcode ID/URI.
        print("-"*30)
        print(colored(
            f"{'Bitcode ID (last 8 characters):':<40} {'File name:':<75}",
            "red"))
        print(f"{bitcode_id[-8:]:<40} {os.path.basename(bitcode_uri):<75}")
        print(colored(
            f"{'Called Function:':<50} {'Return Type:':<30} {'Call Sites:':<8}",
            "green"))

        # Determining if any called functions exist for bitcode file.
        try:
            called_functions = id_called_functions_dict[bitcode_id]
        except KeyError:
            print("NONE FOUND")
            continue

        total_num_call_sites = 0
        # Printing each individual called function.
        for called_function in called_functions:
            function_name = called_function["function"]["sourceName"]
            return_type = called_function["function"]["returnType"]
            call_sites = called_function["totalCallSites"]
            total_num_call_sites += call_sites

            print(f"{function_name:<50} {return_type:<30} {call_sites:<10}")

        print(f"Average number of call sites per function: {total_num_call_sites/len(called_functions)}")

def list_defined_functions(database):
    """Prints out the defined functions for the bitcode files in MongoDB."""

    print("---Defined functions that appear in database---")
    id_file_dict = cli.bitcode.db.read_id_file_dict(database)
    id_defined_functions_dict = cli.bitcode.db.read_id_defined_functions_dict(
        database)

    if not id_defined_functions_dict:
        log.error("No defined functions found for any bitcode file!")
        log.error("Make sure that GetDefinedFunctionsAll "
                  "or GetDefinedFunctionsUri has been ran!")
        sys.exit(1)

    for bitcode_id, bitcode_uri in id_file_dict.items():
        # Printing out header entries for each bitcode ID/URI.
        print("-"*30)
        print(colored(
            f"{'Bitcode ID (last 8 characters):':<40} {'File name:':<75}",
            "red"))
        print(f"{bitcode_id[-8:]:<40} {os.path.basename(bitcode_uri):<75}")
        print(colored(
            f"{'Defined Function:':<50} {'Return Type:':<30}",
            "green"))

        # Determining if any defined functions exist for bitcode file.
        try:
            defined_functions = id_defined_functions_dict[bitcode_id]
        except KeyError:
            print("NONE FOUND")
            continue

        # Printing out each defined function.
        for defined_function in defined_functions:
            function_name = defined_function["sourceName"]
            return_type = defined_function["returnType"]

            print(f"{function_name:<50} {return_type:<30}")
