""" Stores and retrieves data from a MongoDB relevant to bitcode services."""

import json

import google.protobuf.json_format
import glog as log

import cli.common.uri
import cli.db.db
import proto.bitcode_pb2

def read_ids(database):
    """Returns the set of all bitcode IDs in a database as strings."""

    bitcode_ids = set()
    for entry in database.filenames.find():
        bitcode_ids.add(entry["bitcode_id"])

    return bitcode_ids

def read_uris(database):
    """Returns the list of all uris in a database as protobuf messages."""

    uris = []
    for entry in database.filenames.find():
        uri = entry["uri"]
        # Parsing into a protobuf message
        uri = cli.common.uri.parse(uri)
        uris.append(uri)

    return uris

def read_id_file_dict(database):
    """Returns a dictionary from bitcode id to uri string for all bitcode
       files registered and stored in database
    """

    bitcode_id_file = {}
    for entry in database.filenames.find():
        bitcode_file = entry["uri"]
        bitcode_id = entry["bitcode_id"]
        bitcode_id_file[bitcode_id] = bitcode_file
    return bitcode_id_file

def read_id_func_type_dict(database):
    """Returns a dictionary from bitcode id to a dictionary of function type
       to return type for all bitcode files registered in database
    """

    bitcode_id_func_type = {}
    for entry in database.called_functions.find():
        bitcode_id = entry["bitcode_id"]
        return_type = entry["function"]["returnType"]
        func_name = entry["function"]["sourceName"]

        # Checking if bitcode id already has values associated with it
        if bitcode_id in bitcode_id_func_type.keys():
            bitcode_id_func_type[bitcode_id][func_name] = return_type
        else:
            bitcode_id_func_type[bitcode_id] = {func_name: return_type}

    return bitcode_id_func_type

def read_id_defined_functions_dict(database):
    """Returns a dictionary from bitcode id to the list of defined functions."""

    id_defined_functions = {}
    for entry in database.DefinedFunctionsResponse.find():
        bitcode_id = entry["request"]["bitcodeId"]["id"]
        id_defined_functions[bitcode_id] = entry["functions"]

    return id_defined_functions

def read_id_called_functions_dict(database):
    """ Returns a dictionary from bitcode id to the list of called functions
        from the database
    """

    id_called_functions = {}
    for entry in database.CalledFunctionsResponse.find():
        bitcode_id = entry["request"]["bitcodeId"]["id"]
        id_called_functions[bitcode_id] = entry["calledFunctions"]

    return id_called_functions

def read_called_functions_for_id(database, bitcode_id):
    """ Returns called functions for a given bitcode ID. """

    return database.CalledFunctionsResponse.findOne({"request.bitcodeId.id" : bitcode_id})

def read_id_local_called_functions_dict(database):
    """ Returns a dictionary from bitcode id to the list of local called
        functions from the database
    """

    id_local_called_functions = {}
    for entry in database.LocalCalledFunctionsResponse.find():
        bitcode_id = entry["request"]["bitcodeId"]["id"]
        id_local_called_functions[bitcode_id] = entry["localCalledFunctions"]

    return id_local_called_functions

def read_id_file_called_functions_dict(database):
    """ Returns a dictionary from bitcode id to the list of file called
        functions from the database
    """

    id_file_called_functions = {}
    for entry in database.FileCalledFunctionsResponse.find():
        bitcode_id = entry["request"]["bitcodeId"]["id"]
        id_file_called_functions[bitcode_id] = entry["fileCalledFunctions"]

    return id_file_called_functions

def read_id_for_uri(database, uri):
    """Returns the string bitcode id if it exists in database given a uri.

    Args:
        uri: Uri message in protobuf

    Returns:
        The string bitcode ID or None if not found in database.
    """

    str_uri = cli.common.uri.to_str(uri)
    entry = database.filenames.find_one({"uri": str_uri})
    if entry:
        return entry["bitcode_id"]

    return entry

def insert_bitcode(database, bitcode_id, uri):
    """Inserts a bitcode ID and URI into a database.

    Args:
        database: The mongo database client
        bitcode_id: The bitcode ID to associate.
        uri: The URI of the bitcode file.
    """

    str_uri = cli.common.uri.to_str(uri)
    bitcode_id = bitcode_id.id
    if database.filenames.find_one({'bitcode_id': bitcode_id}) is not None:
        log.warn("Bitcode ID already in database.")

    if database.filenames.find_one({'uri': str_uri}) is not None:
        log.error("Attempt to re-use URI already existing in database. "
                  "NOT inserting!")
        return

    database.filenames.insert(
        {"uri": str_uri,
         "bitcode_id": bitcode_id}
    )

def read_called_functions_request(database, bitcode_id):
    """Returns a CalledFunctionsRequest corresponding to bitcode_id.

    Retrieves the CalledFunctionsRequest from the database that pertains
    to the supplied bitcode_id. If no corresponding entry is found, None
    is returned and the caller must handle it.

    Returns:
        A CalledFunctionsRequest proto message corresponding to bitcode_id
        or None if there is no corresponding entry in the database.
    """

    entry = database.CalledFunctionsResponse.find_one(
        {"request.bitcodeId.id": bitcode_id})

    # Caller must handle if there are no entries.
    if not entry:
        return None

    request = entry.pop("request")
    request_json = json.dumps(request)
    message = proto.bitcode_pb2.CalledFunctionsRequest()
    google.protobuf.json_format.Parse(request_json, message)

    return message

def read_called_functions_response(database, bitcode_id):
    """Returns a CalledFunctionsResponse corresponding to bitcode_id.

    Retrieves the CalledFunctionsResponse from the database that pertains
    to the supplied bitcode_id. If no corresponding entry is found, None
    is returned and the caller must handle it.

    Returns:
        A CalledFunctionsResponse proto message corresponding to bitcode_id
        or None if there is no corresponding entry in the database.
    """

    # Querying MongoDB for called functions response.
    entry = database.CalledFunctionsResponse.find_one(
        {"request.bitcodeId.id": bitcode_id})

    # Caller must handle if there are no entries.
    if not entry:
        return None

    # Checking that only one entry exists for a bitcode ID
    # request/response pair.
    entry.pop("_id")
    entry.pop("request")
    entry_json = json.dumps(entry)
    message = proto.bitcode_pb2.CalledFunctionsResponse()
    google.protobuf.json_format.Parse(entry_json, message)

    return message

def read_defined_functions_request(database, bitcode_id):
    """Returns the DefinedFunctionsRequest corresponding to bitcode_id.

    Retrieves the DefinedFunctionsRequest from the database that pertains
    to the supplied bitcode_id. If no corresponding entry is found, None
    is returned and the caller must handle it.

    Returns:
        A DefinedFunctionsRequest proto message corresponding to bitcode_id
        or None if there is no corresponding entry in the database.
    """

    entry = database.DefinedFunctionsResponse.find_one(
        {"request.bitcodeId.id": bitcode_id})

    # Caller must handle if there are no entries.
    if not entry:
        return None

    request = entry.pop("request")
    request_json = json.dumps(request)
    message = proto.bitcode_pb2.DefinedFunctionsRequest()
    google.protobuf.json_format.Parse(request_json, message)

    return message

def read_defined_functions_response(database, bitcode_id):
    """Returns the DefinedFunctionsResponse corresponding to bitcode_id

    Retrieves the DefinedFunctionsResponse from the database that pertains
    to the supplied bitcode_id. If no corresponding entry is found, None
    is returned and the caller must handle it.

    Returns:
        A DefinedFunctionsResponse proto message corresponding to bitcode_id
        or None if there is no corresponding entry in the database.
    """

    entry = database.DefinedFunctionsResponse.find_one(
        {"request.bitcodeId.id": bitcode_id})

    # Caller must handle if there are no entries.
    if not entry:
        return None

    entry.pop("_id")
    entry.pop("request")
    entry_json = json.dumps(entry)
    message = proto.bitcode_pb2.DefinedFunctionsResponse()
    google.protobuf.json_format.Parse(entry_json, message)

    return message

def delete_called_functions_all(database):
    """Drops the entire CalledFunctiosnResponse collection."""
    database.CalledFunctionsResponse.drop()

def delete_local_called_functions_all(database):
    """Drops the entire LocalCalledFunctionsResponse collection."""
    database.LocalCalledFunctionsResponse.drop()

def delete_file_called_functions_all(database):
    """Drops the entire FileCalledFunctionsResponse collection."""
    database.FileCalledFunctionsResponse.drop()

def delete_defined_functions_all(database):
    """Drops the entire DefinedFunctionsResponse collection."""
    database.DefinedFunctionsResponse.drop()

def delete_called_functions(database, bitcode_id_handle):
    """Removes a called functions entry for a given bitcode id."""

    database.CalledFunctionsResponse.remove(
        {"request.bitcodeId.id": bitcode_id_handle.id})

def delete_local_called_functions(database, bitcode_id_handle):
    """Removes a local called functions entry for a given bitcode id."""

    database.LocalCalledFunctionsResponse.remove(
        {"request.bitcodeId.id": bitcode_id_handle.id})

def delete_file_called_functions(database, bitcode_id_handle):
    """Removes a file called functions entry for a given bitcode id."""

    database.FileCalledFunctionsResponse.remove(
        {"request.bitcodeId.id": bitcode_id_handle.id})

def delete_defined_functions(database, bitcode_id_handle):
    """Removes a defined functions entry for a given bitcode id """

    database.DefinedFunctionsResponse.remove(
        {"request.bitcodeId.id": bitcode_id_handle.id})

def delete_filenames_entry(database, bitcode_uri):
    """Removes a filenames entry for a supplied bitcode URI."""

    database.filenames.remove(
        {"uri": cli.common.uri.to_str(bitcode_uri)})

def insert_called_functions(database, request, unpacked_response):
    """Unpacks CalledFunctionsResponse and inserts request/response
       pair into database.

    Args:
        request: CalledFunctionsRequest
        unpacked_response: Unpacked CalledFunctionsResponse
    """
    finished_response = proto.bitcode_pb2.CalledFunctionsResponse()
    unpacked_response.response.Unpack(finished_response)

    cli.db.db.insert_request_response_pair(database, request, finished_response)

def insert_local_called_functions(database, request, unpacked_response):
    """Unpacks LocalCalledFunctionsResponse and inserts request/response
       pair into database.

    Args:
        request: LocalCalledFunctionsRequest
        unpacked_response: Unpacked LocalCalledFunctionsResponse
    """
    finished_response = proto.bitcode_pb2.LocalCalledFunctionsResponse()
    unpacked_response.response.Unpack(finished_response)

    cli.db.db.insert_request_response_pair(database, request, finished_response)

def insert_file_called_functions(database, request, unpacked_response):
    """Unpacks FileCalledFunctionsResponse and inserts request/response
       pair into database.

    Args:
        request: FileCalledFunctionsRequest
        unpacked_response: Unpacked FileCalledFunctionsResponse
    """
    finished_response = proto.bitcode_pb2.FileCalledFunctionsResponse()
    unpacked_response.response.Unpack(finished_response)

    cli.db.db.insert_request_response_pair(database, request, finished_response)

def insert_defined_functions(database, request, unpacked_response):
    """Unpacks DefinedFunctionsResponse and inserts request/response pair
       into database.

    Args:
        request: DefinedFunctionsRequest
        unpacked_response: Unpacked DefinedFunctionsResponse
    """

    finished_response = proto.bitcode_pb2.DefinedFunctionsResponse()
    unpacked_response.response.Unpack(finished_response)

    cli.db.db.insert_request_response_pair(database, request, finished_response)
