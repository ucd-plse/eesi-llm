# Command-line Interface: Bitcode

## Commands

The CLI Bitcode commands are broken into three primary groups: registration,
called functions, and defined functions. The called functions and defined
functions commands currently require that the relevant bitcode files are
already registered with the bitcode service and are correctly stored in
MongoDB.

### Registration

#### Register a single bitcode file

This command registers a single bitcode file with the bitcode service
and stores its information in MongoDB. For example, running the command
on `fopen.ll` in `ErrorSpecifications/testdata/programs`:

```bash
bazel run //cli:main -- bitcode RegisterBitcode --uri file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/fopen.ll --project fopen
```

Running the command will result in a similar entry in the `filenames` collection
in your Mongo database.

```
{
  "_id": ObjectId("..."),
  "project": "fopen",
  "uri": "file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/fopen.ll",
  "bitcode_id": "d8383e4f4ce1ee82fc321f998470dbd48b6a68e3c565c7ef2b3fba195667dea4"
}
```

#### Register a local dataset

This command registers all bitcode files in a local dataset (directory)
with the bitcode service and stores all bitcode files' information in
MongoDB. For example, registering all bitcode files in `ErrorSpecifications/testdata/programs`.

```bash
bazel run //cli:main -- bitcode RegisterLocalDataset --uri file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/
```

Running the command will result in an entry for each bitcode file that exists in 
the dataset. Using the `ErrorSpecifications/testdata/programs` bitcode files should
result in multiple entries, looking similar to:

```
{
  "_id" : ObjectId("..."),
  "project" : "programs",
  "uri" : "file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/hello_twice.ll",
  "bitcode_id" : "0f4601800772ebcb6e8ba00e95f1b3d11be33bab6875e42087f6ee566ae187e8"
},
...
{
  "_id" : ObjectId("..."),
  "project" : "programs",
  "uri" : "file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/calls_ptr.ll",
  "bitcode_id" : "51742d61545392ff92090915956e2d186559296e00a9037aa9206f90384d02b9"
}
```

It is important to note that the `project` field defaults to the directory name where
the bitcode file is located. This simply exists as an easier way to assign projects
to a dataset full of bitcode files.

#### List registered bitcode

This command lists all the bitcode files that are registered and stored in MongoDB.

```bash
bazel run //cli:main -- bitcode ListRegisteredBitcode
```

Sample output for a database with two entries, one for `error_only_function_ptr.ll` and
`hello_twice.ll`.

```bash
---Registered bitcode files in database---
Bitcode ID:                                                                 URI:                                                                       
0f4601800772ebcb6e8ba00e95f1b3d11be33bab6875e42087f6ee566ae187e8            file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/hello_twice.ll
69ff50b41eda48cf5857d1747a23051d67b54017247c822ace83872522126384            file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/error_only_function_ptr.ll
```

### Called Functions

All called functions commands pertain to commands that retrieve called
functions for bitcode files.

#### Called Functions for a Single URI

This command gets all called functions for a single bitcode file given its
URI. For example running on `fopen.ll` in `ErrorSpecifications/testdata/programs`.

```bash
bazel run //cli:main -- bitcode GetCalledFunctionsUri --uri file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/fopen.ll
```

The resulting Mongo database entry for the bitcode file will look similar to:

```
{
  "_id" : ObjectId("..."),
  "calledFunctions" : [
    {
      "function" : {
        "llvmName" : "fopen",
        "sourceName" : "fopen",
        "returnType" : "FUNCTION_RETURN_TYPE_POINTER"
      },
      "totalCallSites" : 1
    }
  ],
  "request" : {
    "bitcodeId" : {
      "id" : "d8383e4f4ce1ee82fc321f998470dbd48b6a68e3c565c7ef2b3fba195667dea4",
      "authority" : "localhost:50051"
    }
  }
}
```


#### Called Functions for all Registered Bitcode

This command gets all called functions for all bitcode files that have their
information stored in MongoDB from bitcode registration. There is no need to
provide a URI to a dataset of any kind as all bitcode files that are registered
with the database will have their URI listed in their entry.

```bash
bazel run //cli:main -- bitcode GetCalledFunctionsAll
```

The output for this command will be the same as the called functions command 
for a single bitcode file. The only difference is that there will now be
multiple entries added.

#### List Called Functions

This command lists all the called functions that are in MongoDB.

```bash
bazel run //cli:main -- bitcode ListCalledFunctions
```

Sample output for a database with two entries, one for `error_only_function_ptr.ll` and
`hello_twice.ll`.

```bash
---Called functions that appear in database---
------------------------------
Bitcode ID:                                                                 URI:                                                                       
0f4601800772ebcb6e8ba00e95f1b3d11be33bab6875e42087f6ee566ae187e8            file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/hello_twice.ll
Called Function:                                   Return Type:                   Call Sites:
printf                                             FUNCTION_RETURN_TYPE_INTEGER   2         
------------------------------
Bitcode ID:                                                                 URI:                                                                       
69ff50b41eda48cf5857d1747a23051d67b54017247c822ace83872522126384            file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/error_only_function_ptr.ll
Called Function:                                   Return Type:                   Call Sites:
error_only                                         FUNCTION_RETURN_TYPE_VOID      1         
malloc                                             FUNCTION_RETURN_TYPE_POINTER   1 
```

### Defined Functions

All defined functions commands pertain to commands that retrieve defined
functions for bitcode files.

#### Defined Functions for a Single URI

This command gets all defined functions for a single bitcode file given its
URI. For example, running on `fopen.ll` in `ErrorSpecifications/testdata/programs`.

```bash
bazel run //cli:main -- bitcode GetDefinedFunctionsUri --uri file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/fopen.ll
```

The resulting Mongo database entry for the bitcode file will look similar to:

```
{
  "_id" : ObjectId("..."),
  "functions" : [
    {
      "llvmName" : "main",
      "sourceName" : "main",
      "returnType" : "FUNCTION_RETURN_TYPE_INTEGER"
    }
  ],
  "request" : {
    "bitcodeId" : {
      "id" : "d8383e4f4ce1ee82fc321f998470dbd48b6a68e3c565c7ef2b3fba195667dea4",
      "authority" : "localhost:50051"
    }
  }
}
```

#### Defined Functions for all Registered Bitcode

This command gets all defined functions for all bitcode files that have their
information stored in MongoDB from bitcode registration. Similarily to the
called functions command, there is no need to provide a URI to a dataset, as
all bitcode files that are already registered in the database will have their
respective URIs as part of their entries.

```bash
bazel run //cli:main -- bitcode GetDefinedFunctionsAll
```

The output for this command will be the same as the defined functions command
for a single bitcode file. The only difference is that there will now be
multiple entries added.

#### List Defined Functions

This command lists all the defined functions that are in MongoDB.

```bash
bazel run //cli:main -- bitcode ListDefinedFunctions
```

Sample output for a database with two entries, one for `error_only_function_ptr.ll` and
`hello_twice.ll`.

```bash
---Defined functions that appear in database---
------------------------------
Bitcode ID:                                                                 URI:                                                                       
0f4601800772ebcb6e8ba00e95f1b3d11be33bab6875e42087f6ee566ae187e8            file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/hello_twice.ll
Defined Function:                                  Return Type:                  
main                                               FUNCTION_RETURN_TYPE_INTEGER  
------------------------------
Bitcode ID:                                                                 URI:                                                                       
69ff50b41eda48cf5857d1747a23051d67b54017247c822ace83872522126384            file:///<PATH_TO_PROJECT>/ErrorSpecifications/testdata/programs/error_only_function_ptr.ll
Defined Function:                                  Return Type:                  
foo                                                FUNCTION_RETURN_TYPE_POINTER 
```
