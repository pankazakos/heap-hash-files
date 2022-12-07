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
  strncpy(ht_info.type, "Hash_file", 10 * sizeof(char));
  ht_info.fileDesc = fd;
  ht_info.numBuckets = buckets;
  // initialize hash table
  int *hash_table = malloc(buckets * sizeof(int));
  for (int i = 0; i < buckets; i++) {
    hash_table[i] = i;
  }
  ht_info.hash_table = hash_table;
  memcpy(ndata, &ht_info, sizeof(HT_info));

  ndata += sizeof(HT_info);
  memcpy(ndata, ht_info.hash_table, buckets * sizeof(int));

  free(hash_table);

  BF_Block_SetDirty(metadata_block);
  CALL_BF(BF_UnpinBlock(metadata_block));
  BF_Block_Destroy(&metadata_block);

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

  int hash_table[ht_info->numBuckets];
  ndata += sizeof(HT_info);
  memcpy(hash_table, ndata, ht_info->numBuckets * sizeof(int));

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
  free(HT_info);
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
