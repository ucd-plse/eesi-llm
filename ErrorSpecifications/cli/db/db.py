"""Generic database helper functions."""
import google.protobuf
import glog as log
import pymongo

def connect(db_name, db_host, db_port):
    """Connect to a MongoDB.

    Args:
        db_name: Name of the database to use.
        db_host: Host address of Mongo DB.
        db_port: Port number for Mongo DB.
    Returns:
        A pymongo client
    """

    client = pymongo.MongoClient(
        db_host, db_port, serverSelectionTimeoutMS=500)[db_name]
    try:
        client.command("ismaster")
    except pymongo.errors.ConnectionFailure:
        log.error("Could not connect to MongoDB!")
        log.error("Make sure that MongoDB is installed and running!")
        log.error("Exiting...")
        exit()

    return client

def insert_message(database, bitcode_id, message):
    """
    Args:
        db: The mongo database object.
        message: The message to insert into the database.
        bitcode_id: String bitcode ID.
    """
    json_message = google.protobuf.json_format.MessageToDict(message)
    if bitcode_id:
        json_message['bitcode_id'] = bitcode_id
    collection = message.DESCRIPTOR.name
    database[collection].insert(json_message)

def insert_request_response_pair(database, request, response):
    """Inserts a request and response pair entry into the DB.

    Inserts a request and response pair entry into a DB collection based on the
    response descriptor's name.

    Args:
    """
    json_request = google.protobuf.json_format.MessageToDict(request)
    json_response = google.protobuf.json_format.MessageToDict(response)
    json_response['request'] = json_request
    collection = response.DESCRIPTOR.name
    database[collection].insert(json_response)

def collection_contains_id(database, id_type, unique_id, collection):
    """Determines if a given collection contains a unique ID.

    Args:
        id_type: The type of ID to look for (e.g. bitcode)
        unique_id: The unique identifier
    Returns:
        True if collection contains the ID
        False if collection does not contain the ID
    """
    query_dict = {id_type: unique_id}

    # If find_one returns a result, bitcode id exists in collection
    if database[collection].find_one(query_dict):
        return True

    return False
