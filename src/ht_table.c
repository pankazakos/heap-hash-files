#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"

#define CALL_BF(call)                                                          \
  {                                                                            \
    BF_ErrorCode code = call;                                                  \
    if (code != BF_OK) {                                                       \
      BF_PrintError(code);                                                     \
      return HT_ERROR;                                                         \
    }                                                                          \
  }

// call functions only in debug mode. Usage d(func())
#ifdef DEBUG
#define d(x) x
#else
#define d(x)
#endif

#define CAPACITY BF_BLOCK_SIZE / sizeof(Record)

int Hash_function(int key, int size) { return key % size; }

int HT_CreateFile(char *fileName, int buckets) {
  // create file
  CALL_BF(BF_CreateFile(fileName));

  // file is opened and closed temporarily inside this function
  int fd;
  CALL_BF(BF_OpenFile(fileName, &fd));

  // create metadata block
  BF_Block *metadata_block;
  BF_Block_Init(&metadata_block);
  CALL_BF(BF_AllocateBlock(fd, metadata_block));

  // store type of file and filedescriptor in metadata
  // sdata: starting point of data (Do not move pointer)
  // ndata: next data (Can move pointer)
  char *sdata = BF_Block_GetData(metadata_block);
  char *ndata = sdata;
  HT_info ht_info;
  memset(&ht_info, 0, sizeof(HT_info));
  strncpy(ht_info.type, "Hash_file", 10 * sizeof(char));
  ht_info.fileDesc = fd;
  ht_info.numBuckets = buckets;
  ht_info.capacity = BF_BLOCK_SIZE / sizeof(Record);
  // initialize hash table
  int *hash_table = malloc(buckets * sizeof(int));
  for (int i = 0; i < buckets; i++) {
    hash_table[i] = i + 1; // skip metadata block
  }
  ht_info.hash_table = hash_table;
  memcpy(ndata, &ht_info, sizeof(HT_info));

  ndata += sizeof(HT_info);
  memcpy(ndata, ht_info.hash_table, buckets * sizeof(int));

  BF_Block_SetDirty(metadata_block);
  CALL_BF(BF_UnpinBlock(metadata_block));
  BF_Block_Destroy(&metadata_block);

  // allocate initial blocks (suppose each starts with one block)
  BF_Block *new_block;
  BF_Block_Init(&new_block);
  for (int i = 0; i < buckets; i++) {
    CALL_BF(BF_AllocateBlock(fd, new_block));

    // store block info
    char *sdata = BF_Block_GetData(new_block);
    char *ndata = sdata;
    ndata += CAPACITY * sizeof(Record);
    HT_block_info block_info;
    block_info.records = 0;
    block_info.overflow_block = -1;
    memcpy(ndata, &block_info, sizeof(HT_block_info));

    // commit changes
    BF_Block_SetDirty(new_block);
    CALL_BF(BF_UnpinBlock(new_block));
  }
  BF_Block_Destroy(&new_block);

  CALL_BF(BF_CloseFile(ht_info.fileDesc));
  return HT_OK;
}

HT_info *HT_OpenFile(char *fileName) {
  // open file
  int fd;
  BF_ErrorCode code = BF_OpenFile(fileName, &fd);
  if (code != BF_OK) {
    BF_PrintError(code);
    return NULL;
  }

  // read metadata block
  BF_Block *metadata_block;
  BF_Block_Init(&metadata_block);
  int first_block = 0;
  code = BF_GetBlock(fd, first_block, metadata_block);
  if (code != BF_OK) {
    BF_PrintError(code);
    return NULL;
  }

  // read HT_info from metadata block
  char *sdata = BF_Block_GetData(metadata_block);
  char *ndata = sdata;
  HT_info *ht_info = malloc(sizeof(HT_info));
  memcpy(ht_info, ndata, sizeof(HT_info));

  // check if file is a hash file
  printf("%s\n", ht_info->type);
  if (strcmp(ht_info->type, "Hash_file")) {
    printf("Given file is not of type Hash_file\n");
    return NULL;
  }

  code = BF_UnpinBlock(metadata_block);
  if (code != BF_OK) {
    BF_PrintError(code);
    return NULL;
  }
  BF_Block_Destroy(&metadata_block);
  return ht_info;
}

int HT_CloseFile(HT_info *HT_info) {
  // close file
  CALL_BF(BF_CloseFile(HT_info->fileDesc));
  free(HT_info->hash_table);
  free(HT_info);
  return HT_OK;
}

int HT_InsertEntry(HT_info *ht_info, Record record) {
  // insert entry
  int bucket = Hash_function(record.id, ht_info->numBuckets);

  // get first block of bucket from hash_table
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(ht_info->fileDesc, ht_info->hash_table[bucket], block));
  char *sdata = BF_Block_GetData(block);
  char *ndata = sdata;
  ndata += CAPACITY * sizeof(Record);

  HT_block_info block_info;
  memcpy(&block_info, ndata, sizeof(HT_block_info));

  if (block_info.records == CAPACITY) {
    // allocate new block and update first block of bucket
    BF_Block *new_block;
    BF_Block_Init(&new_block);
    CALL_BF(BF_AllocateBlock(ht_info->fileDesc, new_block));

    // insert entry
    char *sdata = BF_Block_GetData(new_block);
    char *ndata = sdata;
    memcpy(ndata, &record, sizeof(Record));

    // update block_info
    ndata += CAPACITY * sizeof(Record);
    HT_block_info new_block_info;
    memcpy(&new_block_info, ndata, sizeof(HT_block_info));
    new_block_info.records = 1;
    new_block_info.overflow_block = ht_info->hash_table[bucket];
    memcpy(ndata, &new_block_info, sizeof(HT_block_info));

    BF_Block_SetDirty(new_block);
    CALL_BF(BF_UnpinBlock(new_block));
    BF_Block_Destroy(&new_block);

    // update first block of bucket
    BF_Block *metadata_block;
    BF_Block_Init(&metadata_block);
    CALL_BF(BF_GetBlock(ht_info->fileDesc, 0, metadata_block));
    char *smetadata = BF_Block_GetData(metadata_block);
    char *nmetadata = smetadata;

    nmetadata += sizeof(HT_info);
    int blocks;
    CALL_BF(BF_GetBlockCounter(ht_info->fileDesc, &blocks));
    int new_block_idx = blocks - 1;
    ht_info->hash_table[bucket] = new_block_idx;
    memcpy(nmetadata, ht_info->hash_table, ht_info->numBuckets * sizeof(int));

    BF_Block_SetDirty(metadata_block);
    CALL_BF(BF_UnpinBlock(metadata_block));
    BF_Block_Destroy(&metadata_block);

  } else {
    // insert entry
    ndata = sdata + block_info.records * sizeof(Record);
    memcpy(ndata, &record, sizeof(Record));

    // update block_info
    block_info.records++;
    ndata = sdata + CAPACITY * sizeof(Record);
    memcpy(ndata, &block_info, sizeof(HT_block_info));
    BF_Block_SetDirty(block);
  }

  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);

  return HT_OK;
}

int HT_GetAllEntries(HT_info *ht_info, void *value) {
  // find entries with given value

  int casted_value;
  memcpy(&casted_value, value, sizeof(int));

  // hash value
  int curr_block_idx = Hash_function(casted_value, ht_info->numBuckets);

  // get first block of bucket
  curr_block_idx = ht_info->hash_table[curr_block_idx];

  // sequential search in bucket
  // search first block and then the remaining blocks of bucket
  while (1) {
    BF_Block *curr_block;
    BF_Block_Init(&curr_block);
    CALL_BF(BF_GetBlock(ht_info->fileDesc, curr_block_idx, curr_block));
    char *sdata = BF_Block_GetData(curr_block);
    char *ndata = sdata;
    ndata += CAPACITY * sizeof(Record);

    // read block info
    HT_block_info block_info;
    memcpy(&block_info, ndata, sizeof(HT_block_info));

    // sequential search in block
    for (int i = 0; i < block_info.records; i++) {
      ndata = sdata + i * sizeof(Record);
      Record record;
      memcpy(&record, ndata, sizeof(Record));
      if (record.id == casted_value) {
        Record helper;
        strncpy(helper.record, "Record", 7 * sizeof(char));
        char *id = malloc(3 * sizeof(char));
        strncpy(id, "ID", 3 * sizeof(char));
        strncpy(helper.name, "Name", 5 * sizeof(char));
        strncpy(helper.surname, "Surname", 8 * sizeof(char));
        strncpy(helper.city, "City", 5 * sizeof(char));

        printf("%-10s%-10s%-15s%-20s%-20s\n", helper.record, id, helper.name,
               helper.surname, helper.city);

        free(id);

        printf("%-10s%-10d%-15s%-20s%-20s\n", record.record, record.id,
               record.name, record.surname, record.city);

        // return since assuming that file does not contain duplicates
        CALL_BF(BF_UnpinBlock(curr_block));
        BF_Block_Destroy(&curr_block);
        return HT_OK;
      }
    }
    curr_block_idx = block_info.overflow_block;

    // return if there is no overflow block left
    if (curr_block_idx == -1) {
      CALL_BF(BF_UnpinBlock(curr_block));
      BF_Block_Destroy(&curr_block);
      return HT_OK;
    }
    CALL_BF(BF_UnpinBlock(curr_block));
    BF_Block_Destroy(&curr_block);
  }
}
