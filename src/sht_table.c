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
  return SHT_OK;
}

int SHT_CloseSecondaryIndex(SHT_info *SHT_info) {
  // shtClose

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