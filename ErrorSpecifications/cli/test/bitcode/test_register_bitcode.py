""" Tests for RegisterBitcode commands """

import os
import sys

import pytest

import cli.bitcode.commands
import cli.common.service_configuration_handler as service_handler
import cli.test.common.common as common

BITCODE_HOST = "127.0.0.1"
# This port number is unique to this test file. This is also only the base port
# number, as we utilize multiple port numbers while testing. Each test in this
# file will use a unique port number (incrementing starting from this base
# number). This file should only utlize 60090-60099.
BASE_BITCODE_PORT = 60090

def test_register_bitcode():
    """Tests registering a bitcode file with the bitcode service."""
    # Getting a unique bitcode port for this test, starting from
    # BASE_BITCODE_PORT.
    bitcode_port = str(BASE_BITCODE_PORT)
    # Getting a mock database and launching the bitcode service.
    database = common.setup_database()
    common.setup_service(BITCODE_HOST, bitcode_port, "bitcode")

    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=BITCODE_HOST, bitcode_port=bitcode_port)

    # String representation of the bitcode file URI in the test dataset.
    bitcode_uri = os.path.join(common.STR_DATASET_URI, "fopen.ll")
    # Bitcode IDs are explained in cli/bitcode/test/common.py.
    bitcode_id = \
        "88c838911d52c96f06a4083cfc27bf3722ca754b53e9179a4bdd9503a5f9e6c8"

    # Registering fopen.ll with the bitcode service.
    cli.bitcode.commands.register_bitcode(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Checking if the bitcode's information was correctly stored in the
    # database after registration.
    registered_entry = database.filenames.find_one(
        {"uri": bitcode_uri,
         "bitcode_id": bitcode_id,
        }
    )

    # The find_one() function should make these all true. However,
    # to avoid too many assumptions, we check that each field matches
    # the expected output.
    assert registered_entry["uri"] == bitcode_uri
    assert registered_entry["bitcode_id"] == bitcode_id

def test_register_bitcode_reg2mem():
    """Tests registering a bitcode file with the bitcode service."""
    # Getting a mock database and launching the bitcode service.
    # Getting a unique bitcode port for this test, starting from
    # BASE_BITCODE_PORT.
    bitcode_port = str(BASE_BITCODE_PORT + 1)
    database = common.setup_database()
    common.setup_service(BITCODE_HOST, bitcode_port, "bitcode")

    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=BITCODE_HOST, bitcode_port=bitcode_port)

    # String representation of the bitcode file URI in the test dataset.
    bitcode_uri = os.path.join(common.STR_DATASET_URI, "fopen-reg2mem.ll")
    # Bitcode IDs are explained in cli/bitcode/test/common.py.
    bitcode_id = \
        "abd7ca770f9989b5a658702daeb84c2764f7ccd127d3f1d7cc4ac9b6a108c554"

    # Registering fopen.ll with the bitcode service.
    cli.bitcode.commands.register_bitcode(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Checking if the bitcode's information was correctly stored in the
    # database after registration.
    registered_entry = database.filenames.find_one(
        {"uri": bitcode_uri,
         "bitcode_id": bitcode_id,
        }
    )

    # The find_one() function should make these all true. However,
    # to avoid too many assumptions, we check that each field matches
    # the expected output.
    assert registered_entry["uri"] == bitcode_uri
    assert registered_entry["bitcode_id"] == bitcode_id

def test_register_local_dataset():
    """Tests registering all bitcode files in testdata/programs."""
    # Getting a unique bitcode port for this test, starting from
    # BASE_BITCODE_PORT.
    bitcode_port = str(BASE_BITCODE_PORT + 2)
    # Getting a mock database and launching the bitcode service.
    database = common.setup_database()
    common.setup_service(BITCODE_HOST, bitcode_port, "bitcode")

    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=BITCODE_HOST, bitcode_port=bitcode_port)

    # Registering all bitcode files in the test dataset.
    cli.bitcode.commands.register_local_dataset(
        database=database,
        service_configuration_handler=service_configuration_handler,
        uri=common.STR_DATASET_URI,
        overwrite=False,
    )

    # Ensuring that the number of entries matches the number of bitcode
    # files in the dataset.
    assert database.filenames.find().count() is len(common.TESTDATA_INFO)

    # Checking that each bitcode file's entry contains the
    # expected information.
    for bitcode_file, bitcode_id in common.TESTDATA_INFO:
        # String representation of the bitcode file URI.
        bitcode_uri = os.path.join(common.STR_DATASET_URI, bitcode_file)

        # Checking if the bitcode's information was correctly stored in the
        # database after registration.
        registered_entry = database.filenames.find_one(
            {"uri": bitcode_uri,
             "bitcode_id": bitcode_id,
            }
        )

        # The find_one() function should make these all true. However,
        # to avoid too many assumptions, we check that each field matches
        # the expected output.
        assert registered_entry["uri"] == bitcode_uri
        assert registered_entry["bitcode_id"] == bitcode_id

def test_register_duplicate_bitcode():
    """Tests attempting to register fopen.ll twice."""
    # Getting a unique bitcode port for this test, starting from
    # BASE_BITCODE_PORT.
    bitcode_port = str(BASE_BITCODE_PORT + 3)
    # Getting a mock database and launching the bitcode service.
    database = common.setup_database()
    common.setup_service(BITCODE_HOST, bitcode_port, "bitcode")

    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=BITCODE_HOST, bitcode_port=bitcode_port)

    # String representation of the bitcode file URI in the test dataset.
    bitcode_uri = os.path.join(common.STR_DATASET_URI, "fopen.ll")
    # Bitcode IDs are explained in cli/bitcode/test/common.py.
    bitcode_id = \
        "88c838911d52c96f06a4083cfc27bf3722ca754b53e9179a4bdd9503a5f9e6c8"

    # Registering the fopen.ll with the bitcode service.
    cli.bitcode.commands.register_bitcode(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Retrieving number of entries of registered bitcode files before
    # the duplicated command.
    registered_entry_count = database.filenames.find(
        {"uri": bitcode_uri,
         "bitcode_id": bitcode_id,
        }
    ).count()

    # Ensuring that the number of entries before a duplicated registration
    # attempt is 1.
    assert registered_entry_count == 1

    # Attempting to duplicate the registration of fopen.ll.
    cli.bitcode.commands.register_bitcode(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Retrieving number of entries after an attempted registration duplication.
    registered_entry_count = database.filenames.find(
        {"uri": bitcode_uri,
         "bitcode_id": bitcode_id,
        }
    ).count()

    # Ensuring that the duplicated command does not result in a duplicate entry.
    assert registered_entry_count == 1

def test_register_duplicate_bitcode_reg2mem():
    """Tests attempting to register fopen.ll twice."""
    # Getting a unique bitcode port for this test, starting from
    # BASE_BITCODE_PORT.
    bitcode_port = str(BASE_BITCODE_PORT + 4)
    # Getting a mock database and launching the bitcode service.
    database = common.setup_database()
    common.setup_service(BITCODE_HOST, bitcode_port, "bitcode")

    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=BITCODE_HOST, bitcode_port=bitcode_port)

    # String representation of the bitcode file URI in the test dataset.
    bitcode_uri = os.path.join(common.STR_DATASET_URI, "fopen-reg2mem.ll")
    # Bitcode IDs are explained in cli/bitcode/test/common.py.
    bitcode_id = \
        "abd7ca770f9989b5a658702daeb84c2764f7ccd127d3f1d7cc4ac9b6a108c554"

    # Registering the fopen.ll with the bitcode service.
    cli.bitcode.commands.register_bitcode(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Retrieving number of entries of registered bitcode files before
    # the duplicated command.
    registered_entry_count = database.filenames.find(
        {"uri": bitcode_uri,
         "bitcode_id": bitcode_id,
        }
    ).count()

    # Ensuring that the number of entries before a duplicated registration
    # attempt is 1.
    assert registered_entry_count == 1

    # Attempting to duplicate the registration of fopen.ll.
    cli.bitcode.commands.register_bitcode(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Retrieving number of entries after an attempted registration duplication.
    registered_entry_count = database.filenames.find(
        {"uri": bitcode_uri,
         "bitcode_id": bitcode_id,
        }
    ).count()

    # Ensuring that the duplicated command does not result in a duplicate entry.
    assert registered_entry_count == 1

if __name__ == "__main__":
    # To view standard output, add a "-s" to args.
    sys.exit(pytest.main(args=[__file__]))
