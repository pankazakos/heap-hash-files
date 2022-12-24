#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"
#include "sht_table.h"

#define CALL_BF(call)                                                          \
  {                                                                            \
    BF_ErrorCode code = call;                                                  \
    if (code != BF_OK) {                                                       \
      BF_PrintError(code);                                                     \
      return SHT_ERROR;                                                        \
    }                                                                          \
  }

#define MAX_TUPLES BF_BLOCK_SIZE / sizeof(Tuple)

int Hash_name(char *name, int size) {
  int sum = 0;
  int c;
  while ((c = *name++) != 0) {
    sum += c;
  }

  return sum % size;
}

int SHT_CreateSecondaryIndex(char *sfileName, int buckets, char *fileName) {
  // shtCreate

  // create secondary index
  CALL_BF(BF_CreateFile(sfileName));

  // open secondary index
  int fd;
  CALL_BF(BF_OpenFile(sfileName, &fd));

  // create metadata block
  BF_Block *metadata_block;
  BF_Block_Init(&metadata_block);
  CALL_BF(BF_AllocateBlock(fd, metadata_block));

  // store secondary index info in metadata block
  SHT_info sht_info;
  sht_info.fileDesc = fd;
  sht_info.numBuckets = buckets;
  sht_info.hash_table = malloc(buckets * sizeof(int));

  // store sht_info struct
  char *metadata = BF_Block_GetData(metadata_block);
  memcpy(metadata, &sht_info, sizeof(SHT_info));

  // initialize and store hash table
  for (int i = 0; i < buckets; i++) {
    sht_info.hash_table[i] = i + 1; // skip metadata block
  }
  metadata += sizeof(SHT_info);
  memcpy(metadata, sht_info.hash_table, buckets * sizeof(int));

  // commit changes
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
    ndata += MAX_TUPLES * sizeof(Tuple);
    SHT_block_info block_info;
    block_info.tuples = 0;
    block_info.overflow_block = -1;
    memcpy(ndata, &block_info, sizeof(SHT_block_info));

    // commit changes
    BF_Block_SetDirty(new_block);
    CALL_BF(BF_UnpinBlock(new_block));
  }
  BF_Block_Destroy(&new_block);

  // close file
  CALL_BF(BF_CloseFile(fd));

  return SHT_OK;
}

SHT_info *SHT_OpenSecondaryIndex(char *indexName) {
  // shtOpen

  // open file
  int fd;
  BF_ErrorCode code = BF_OpenFile(indexName, &fd);
  if (code != BF_OK) {
    BF_PrintError(code);
    return NULL;
  }

  // read metadata of file
  SHT_info *sht_info = malloc(sizeof(SHT_info));
  BF_Block *metadata_block;
  BF_Block_Init(&metadata_block);
  code = BF_GetBlock(fd, 0, metadata_block);
  if (code != BF_OK) {
    BF_PrintError(code);
    return NULL;
  }

  char *metadata = BF_Block_GetData(metadata_block);
  memcpy(sht_info, metadata, sizeof(SHT_info));

  // Update fileDescriptor
  sht_info->fileDesc = fd;
  memcpy(metadata, sht_info, sizeof(SHT_info));

  code = BF_UnpinBlock(metadata_block);
  if (code != BF_OK) {
    BF_PrintError(code);
    return NULL;
  }
  BF_Block_Destroy(&metadata_block);

  return sht_info;
}

int SHT_CloseSecondaryIndex(SHT_info *sht_info) {
  // shtClose
  CALL_BF(BF_CloseFile(sht_info->fileDesc));
  free(sht_info->hash_table);
  free(sht_info);
  return SHT_OK;
}

int SHT_SecondaryInsertEntry(SHT_info *sht_info, Record record, int block_id) {
  // shtInsert

  int bucket = Hash_name(record.name, sht_info->numBuckets);

  Tuple tuple;
  memset(&tuple, 0, sizeof(Tuple));
  strcpy(tuple.name, record.name);
  tuple.block_id = block_id;

  // get first block of bucket from hash_table
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(sht_info->fileDesc, sht_info->hash_table[bucket], block));

  // read block_info
  char *sdata = BF_Block_GetData(block);
  char *ndata = sdata;
  SHT_block_info block_info;
  ndata += MAX_TUPLES * sizeof(Tuple);
  memcpy(&block_info, ndata, sizeof(SHT_block_info));

  if (block_info.tuples == MAX_TUPLES) {
    // Allocate new block and update first block of bucket
    BF_Block *new_block;
    BF_Block_Init(&new_block);
    CALL_BF(BF_AllocateBlock(sht_info->fileDesc, new_block));

    // insert tuple
    char *sdata = BF_Block_GetData(new_block);
    char *ndata = sdata;
    memcpy(ndata, &tuple, sizeof(Tuple));

    // update block info
    ndata += MAX_TUPLES * sizeof(Tuple);
    SHT_block_info new_block_info;
    memcpy(&new_block_info, ndata, sizeof(SHT_block_info));
    new_block_info.tuples++;
    new_block_info.overflow_block = sht_info->hash_table[bucket];
    memcpy(ndata, &new_block_info, sizeof(SHT_block_info));

    BF_Block_SetDirty(new_block);
    CALL_BF(BF_UnpinBlock(new_block));
    BF_Block_Destroy(&new_block);

    // update first block of bucket
    BF_Block *metadata_block;
    BF_Block_Init(&metadata_block);
    CALL_BF(BF_GetBlock(sht_info->fileDesc, 0, metadata_block));
    char *metadata = BF_Block_GetData(metadata_block);

    metadata += sizeof(SHT_info);
    int blocks;
    CALL_BF(BF_GetBlockCounter(sht_info->fileDesc, &blocks));
    int new_block_idx = blocks - 1;
    sht_info->hash_table[bucket] = new_block_idx;
    memcpy(metadata, sht_info->hash_table, sht_info->numBuckets * sizeof(int));

    BF_Block_SetDirty(metadata_block);
    CALL_BF(BF_UnpinBlock(metadata_block));
    BF_Block_Destroy(&metadata_block);

  } else {
    // insert entry
    ndata = sdata + block_info.tuples * sizeof(Tuple);
    memcpy(ndata, &tuple, sizeof(Tuple));

    // update block info
    block_info.tuples++;
    ndata = sdata + MAX_TUPLES * sizeof(Tuple);
    memcpy(ndata, &block_info, sizeof(SHT_block_info));
    BF_Block_SetDirty(block);
  }

  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);

  return SHT_OK;
}

int SHT_SecondaryGetAllEntries(HT_info *ht_info, SHT_info *sht_info,
                               char *name) {
  // shtFind
  return SHT_OK;
}

int SHT_HashStatistics(char *filename) {
  // Hash Statistics

  // open file
  int fd;
  CALL_BF(BF_OpenFile(filename, &fd));

  // Read metadata of file
  BF_Block *metadata_block;
  BF_Block_Init(&metadata_block);
  CALL_BF(BF_GetBlock(fd, 0, metadata_block));
  char *metadata = BF_Block_GetData(metadata_block);

  SHT_info sht_info;
  memcpy(&sht_info, metadata, sizeof(SHT_info));

  metadata += sizeof(SHT_info);
  int htable_size = sht_info.numBuckets * sizeof(int);
  int *hash_table = malloc(htable_size);
  memcpy(hash_table, metadata, htable_size);

  CALL_BF(BF_UnpinBlock(metadata_block));
  BF_Block_Destroy(&metadata_block);

  int block_counter;
  CALL_BF(BF_GetBlockCounter(fd, &block_counter));

  int min = __INT_MAX__;
  int max = -1;
  int sum_tuples = 0;
  int overflow_buckets = 0;
  int *overflow_blocks = calloc(sht_info.numBuckets, sizeof(int));

  // for each bucket of hash_table
  for (int i = 0; i < sht_info.numBuckets; i++) {
    int bucket_tuples = 0;
    int curr_index = hash_table[i];
    int overflow_flag = 0;

    // for each block of bucket
    while (1) {
      // Read info of current block
      BF_Block *curr_block;
      BF_Block_Init(&curr_block);
      CALL_BF(BF_GetBlock(fd, curr_index, curr_block));
      char *data = BF_Block_GetData(curr_block);
      data += MAX_TUPLES * sizeof(Tuple);
      SHT_block_info block_info;
      memcpy(&block_info, data, sizeof(SHT_block_info));

      bucket_tuples += block_info.tuples;

      CALL_BF(BF_UnpinBlock(curr_block));
      BF_Block_Destroy(&curr_block);

      // stop if there are no overflow blocks left
      if (block_info.overflow_block == -1) {
        break;
      }
      overflow_flag = 1;
      overflow_blocks[i]++;
      curr_index = block_info.overflow_block;
    }

    if (overflow_flag) {
      overflow_buckets++;
    }

    sum_tuples += bucket_tuples;
    if (bucket_tuples < min) {
      min = bucket_tuples;
    } else if (bucket_tuples > max) {
      max = bucket_tuples;
    }
  }

  printf(
      "-------------------------------------------------------------------\n");
  printf("SHT_HashStatistics\n\n");
  printf("Total blocks: %d\n", block_counter);
  printf("Minimum bucket tuples: %d\n", min);
  printf("Maximum bucket tuples: %d\n", max);
  printf("Average bucket tuples: %lf\n",
         (double)sum_tuples / (double)sht_info.numBuckets);
  printf("Average blocks of buckets: %lf\n",
         (double)block_counter / (double)sht_info.numBuckets);
  printf(
      "-------------------------------------------------------------------\n");
  printf("Buckets with overflow blocks: %d\n", overflow_buckets);

  printf("Overflow blocks for each bucket\n");
  for (int i = 0; i < sht_info.numBuckets; i++) {
    if (overflow_blocks[i]) {
      printf("Bucket %d:  %d\n", i, overflow_blocks[i]);
    }
  }
  printf("\n");
  free(hash_table);
  free(overflow_blocks);
  // close file
  CALL_BF(BF_CloseFile(fd));
  return HT_OK;
}