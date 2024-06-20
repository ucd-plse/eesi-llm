"""For storing and retrieving specifications from MongoDB."""
import json

import google.protobuf.json_format
import glog as log

import cli.db.db
import proto.eesi_pb2

def insert_specifications(database, request, finished_response):
    """Inserts specifications for a bitcode file into the database.

    Args:
        database: Pymongo database object used for storing specifications.
        request: GetSpecificationsRequest.
        unpacked_response: Unpacked GetSpecificationsResponse.
    """

    get_specifications_response = proto.eesi_pb2.GetSpecificationsResponse()
    finished_response.response.Unpack(get_specifications_response)
    cli.db.db.insert_request_response_pair(
        database, request, get_specifications_response)
    log.info("Specifications for {} stored in database."
             .format(request.bitcode_id.id))

def insert_specifications_response(database, request, response):

    cli.db.db.insert_request_response_pair(
        database, request, response)
    log.info(f"Specifications for {request.bitcode_id.id} stored in database.")

def delete_specifications(database, bitcode_id):
    """Deletes specifications associated with bitcode_id from database."""

    database.GetSpecificationsResponse.delete_many(
        {"request.bitcodeId.id": bitcode_id})

def read_all_specifications(database):
    """Returns a request/response tuple list for specifications in database."""

    request_specs = []
    for entry in database.GetSpecificationsResponse.find():
        request = entry["request"]
        entry.pop("request")
        entry.pop("_id")

        request = google.protobuf.json_format.ParseDict(
            request, proto.eesi_pb2.GetSpecificationsRequest())
        spec = google.protobuf.json_format.ParseDict(
            entry, proto.eesi_pb2.GetSpecificationsResponse()).specifications
        request_specs.append((request, spec))

    return request_specs

def read_specifications_response(database, bitcode_id):
    """Retrieves a GetSpecificationsResponse for a bitcode ID from database."""

    entry = database.GetSpecificationsResponse.find_one(
        {"request.bitcodeId.id": bitcode_id})
    if not entry:
        return None

    entry.pop("_id")
    entry.pop("request")
    entry_json = json.dumps(entry)
    get_specifications_response = proto.eesi_pb2.GetSpecificationsResponse()
    google.protobuf.json_format.Parse(entry_json, get_specifications_response)

    return get_specifications_response.specifications

def read_specifications_response_dict(database, bitcode_id):
    """Returns a function to specification dictionary for a single entry.

    This function returns a function name, as a string, to a function
    specification lattice element, which is an enum value. If the caller
    wishes for a string representation of the lattice element, they
    must handle that themselves. If no response exists in the database,
    then an empty dictionary is returned.

    Args:
        database: pymongo database object.
        bitcode_id: Bitcode ID associated with GetSpecificationsResponse that
            is used to populate dictionary.
    """

    get_specifications_response = read_specifications_response(
        database, bitcode_id)
    if not get_specifications_response:
        return {}

    return {function_specification.function.source_name:
            function_specification.lattice_element
            for function_specification
            in get_specifications_response}

def read_specifications_request(database, bitcode_id):
    """Retrieves a GetSpecificationsRequest for a bitcode ID from database."""

    entry = database.GetSpecificationsResponse.find_one(
        {"request.bitcodeId.id": bitcode_id})
    if not entry:
        return None

    request_entry = entry["request"]
    entry_json = json.dumps(request_entry)
    get_specifications_request = proto.eesi_pb2.GetSpecificationsRequest()
    google.protobuf.json_format.Parse(entry_json, get_specifications_request)

    return get_specifications_request
