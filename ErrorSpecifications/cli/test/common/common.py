"""Common code used between bitcode CLI tests"""

from hashlib import sha256
import os
import select
import subprocess

import mongomock

# TESTDATA_INFO includes both the bitcode file name and the associated
# bitcode ID, which is the SHA256 hash in our case.
TESTDATA_INFO = []
for filename in os.listdir("testdata/programs/"):
    filename = os.fsdecode(filename)
    file_path = os.path.join("testdata/programs/", filename)
    if not file_path.endswith(".ll"):
        continue
    with open(file_path, 'rb') as file:
        TESTDATA_INFO.append(
            (os.fsdecode(filename), sha256(file.read()).hexdigest()))

# The bitcode IDs are the SHA256 hashes of the bitcode files in
# testdata/programs. They are considered the unique identifier for
# bitcode files. Hashes for a bitcode file can be calculated without
# relying on the CLI using the sha256sum command in Linux e.g.:
#
# $ sha256sum error_code.ll
# $ 5e3b5cf745d18d422da54e20ed5e7f023c805f0b284b379c765a459432511614  error_code.ll
BITCODE_IDS = [
    bitcode_info[1] for bitcode_info in TESTDATA_INFO
]

# String representation of the URI that points to the test bitcode.
STR_DATASET_URI = "file://{}".format(os.path.abspath("testdata/programs/"))

# MongoDB configuration information representing typical settings.
DB_HOST = "localhost"
DB_PORT = 27017
DB_NAME = "error_specifications"

# Services that can be launched for testing.
SERVICES = [
    "bitcode",
    "eesi",
    "checker",
    "getgraph",
]

def setup_services(services):
    """Starts all services as specified and returns a mock MongoDB."""

    for service in services:
        setup_service(**service)

    return setup_database()

def setup_database():
    """Returns a mock MongoDB for testing purposes."""

    return mongomock.MongoClient(DB_HOST, DB_PORT)[DB_NAME]

def setup_service(host, port, service):
    """Launches the bitcode service for tests to use."""

    # Ensuring service is in list of usable services.
    assert service in SERVICES

    # Command to launch the service.
    exec_service = [
        "{}/main".format(service),
        "--listen",
        "{}:{}".format(host, port),
    ]

    # Executing the bitcode service process.
    proc_service = subprocess.Popen(
        exec_service,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    # This is the expected output if the service successfully launches.
    success_status = "Server listening on {}:{}".format(host, port)
    finished = False
    while not finished:
        # Asserting that the bitcode service did not die.
        assert not proc_service.poll()

        ret = select.select(
            [proc_service.stdout.fileno(),
             proc_service.stderr.fileno()],
            [],
            [],
        )

        # Checking the bitcode service process status.
        for f in ret[0]:
            if f == proc_service.stdout.fileno():
                current_status = \
                    proc_service.stdout.readline().decode("utf-8").strip()
                # If the success status appears in the current status, the
                # bitcode service was successful in launching.
                if success_status in current_status:
                    finished = True
                    break
            # Checking for any error output.
            if f == proc_service.stderr.fileno():
                assert not proc_service.stderr.readline()
