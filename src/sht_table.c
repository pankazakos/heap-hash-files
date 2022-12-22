#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"
#include "sht_table.h"

#define CALL_OR_DIE(call)                                                      \
  {                                                                            \
    BF_ErrorCode code = call;                                                  \
    if (code != BF_OK) {                                                       \
      BF_PrintError(code);                                                     \
      exit(code);                                                              \
    }                                                                          \
  }

int SHT_CreateSecondaryIndex(char *sfileName, int buckets, char *fileName) {
  // shtCreate
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

int HashStatistcs(char *filename) {
  // Statistics
  return SHT_OK;
}