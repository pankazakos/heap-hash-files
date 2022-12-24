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

int Hash_Function(char *name, int size) {
  int sum = 0;
  int c;
  while (c = *name++) {
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
  sht_info.hash_table = calloc(buckets, sizeof(int));
  char *metadata = BF_Block_GetData(metadata_block);
  memcpy(metadata, &sht_info, sizeof(SHT_info));

  // commit changes
  BF_Block_SetDirty(metadata_block);
  CALL_BF(BF_UnpinBlock(metadata_block));
  BF_Block_Destroy(&metadata_block);

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
  return SHT_OK;
}

int SHT_SecondaryGetAllEntries(HT_info *ht_info, SHT_info *sht_info,
                               char *name) {
  // shtFind
  return SHT_OK;
}