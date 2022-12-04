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

int HT_CreateFile(char *fileName, int buckets) {
  // create file
  return HT_OK;
}

HT_info *HT_OpenFile(char *fileName) {
  // open file
  return NULL;
}

int HT_CloseFile(HT_info *HT_info) {
  // close file
  return HT_OK;
}

int HT_InsertEntry(HT_info *ht_info, Record record) {
  // insert entry
  return HT_OK;
}

int HT_GetAllEntries(HT_info *ht_info, void *value) {
  // get all entries
  return HT_OK;
}
