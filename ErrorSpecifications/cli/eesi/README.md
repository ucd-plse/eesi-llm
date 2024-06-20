# Command-line Interface: EESI 

## Commands

The CLI EESI commands are currently only concerned with getting
specifications for a bitcode file. Running the commands assumes
that the bitcode of interest is already registered with the
bitcode service, the bitcode information is in the database,
and the called functions for the bitcode file are also stored
in the database. To view the commands associated with these
requirements, go [here](../bitcode/README.md).

### Get Specifications 

#### Get specifications for a single bitcode file

This command gets specifications and violations for a single bitcode file from
the EESI service and inserts the request/response pair into MongoDB.
For example, getting specifications for a bitcode file `fopen.ll`
with the initial specifications in `initial_specifications.txt`:

```bash
bazel run //cli:main -- eesi GetSpecifications --uri file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/fopen.ll --initial-specifications <PATH_TO_PROJECT>/ErrorSpecifications/testdata/domain_knowledge/initial-specifications-test.txt
```

You can also use the `--error-only` and `--error-codes` flags to specify a file
that contains either the error-only functions or error-codes. Running the above command
would result in a similar entry in MongoDB.

```
{
    "_id" : ObjectId("..."),
    "specifications" : [
        {
            "function" : {
                "llvmName" : "main",
                "sourceName" : "main"
            },
            "latticeElement" : "SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO"
        },
        {
            "function" : {
                "llvmName" : "fopen",
                "sourceName" : "fopen"
            },
            "latticeElement" : "SIGN_LATTICE_ELEMENT_ZERO"
        }
    ],
    "request" : {
        "bitcodeId" : {
            "id" : "d8383e4f4ce1ee82fc321f998470dbd48b6a68e3c565c7ef2b3fba195667dea4",
            "authority" : "localhost:50051"
        },
        "initialSpecifications" : [
            {
                "function" : {
                    "llvmName" : "fopen",
                    "sourceName" : "fopen"
                },
                "latticeElement" : "SIGN_LATTICE_ELEMENT_ZERO"
            }
        ]
    }
}
```

#### Get specifications for all bitcode files registered in MongoDB

This command gets specifications and violations for all bitcode files that are
registered in MongoDB. For example, a command to get specifications for all
bitcode files registered in MongoDB.

```bash
bazel run //cli:main -- eesi GetSpecificationsAll --initial-specifications <PATH_TO_PROJECT>/ErrorSpecifications/testdata/domain_knowledge/initial-specifications-test.txt 
```

Again, you can use the `error-only` and `error-codes` flags to specify a file
that contains either the error-only functions or error-codes. The output of
the above command would be similar to the other get specifications command,
the only difference is that there would be multiple entries, one for each
bitcode file that is already registered in MongoDB.

#### List specifications

This command lists the specifications that are currently stored in MongoDB.

```bash
bazel run //cli:main eesi ListSpecifications
```

Sample output for a database with one entry for `error_only_function_ptr.ll`:

```bash
---Specifications that appear in database---
Yellow indicates specification was part of domain knowledge!!!
------------------------------
Bitcode ID:                                                                 URI:                                                                       
69ff50b41eda48cf5857d1747a23051d67b54017247c822ace83872522126384            file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/error_only_function_ptr.ll
Function:                                          Specification:                
foo                                                ==0                           
malloc                                             ==0 
```

In the above output, the text for malloc would be colored yellow as it was used as part of the domain
knowledge when getting specifications. This would be true for all specifications that were
part of the domain knowledge.
