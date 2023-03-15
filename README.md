# heap-hash-files
## Description
Heap-hash-files is a C library with DBMS related functions that can create and store records in heap and hash files. A heap file is a file to which we insert new records consecutively at the end of the file and search linearly. A hash file hashes records based on a field, so that we can efficiently search records. Both heap and hash file support only one table of a given database. Source code is based on BF library (`./lib/libbf.so`) that we were given from our instuctors. This was an assignment of DBMS implementation class of 2022-2023.

## Build project
```
make
```

## Tests
```
make run-hp
make run-ht
make run-sht
make run-main
```

## Metadata
Both heap and hash files have one block of metadata as their first block. In metadata we store information such as the type of the file, the number of maximum records for blocks and the number of buckets.

## Heap files
Every new record that has to be inserted, is inserted at the current last block of the file. If this block is full, then a new block will be created. It provides linear search based on the primary key of a table. When the record with the corresponding key is found, search function returns the number of blocks searched.  

## Hash files
Hash files insert new records to blocks with ids that match the hash value of a field. Each hash file has one primary index and can have many secondary indexes. Primary and secondary indexes spead up the search operations on a file compared to a heap file. If a bucket is full new overflow blocks will be created for this bucket, although there is no rehashing after a limit of overflow blocks. Overflow blocks are reversed each time a new block is added.
### Primary index
Primary index (hash table) is stored in the remaining space of the metadata block, thus there is a limit of how many buckets will fit. The remaining blocks of the hash file are data blocks. It hashes records by the primary key of the database table with a simple hash function. 

### Secondary index
Secondary index uses its own hash table to efficiently search its custom tuples. Tuples store the block id of data block for each value of the field that is is being indexed. Tuples are stored on a different file from data blocks. Secondary indexes are useful to search through records based on other fields rather than the primary key.


### Statistics for hash files
Statistics may be useful for your database hash files since they provide information such as the average, max and min records of buckets. Statistics can be calculated only after the file has been closed.
```
make run-stat
``` 
