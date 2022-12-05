#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call)                                                          \
  {                                                                            \
    BF_ErrorCode code = call;                                                  \
    if (code != BF_OK) {                                                       \
      BF_PrintError(code);                                                     \
      return HP_ERROR;                                                         \
    }                                                                          \
  }

// call functions only in debug mode. Usage d(func())
#ifdef DEBUG
#define d(x) x
#else
#define d(x)
#endif

#define CAPACITY BF_BLOCK_SIZE / sizeof(Record)

int HP_CreateFile(char *fileName) {
  // create file
  CALL_BF(BF_CreateFile(fileName));
  return HP_OK;
}

HP_info *HP_OpenFile(char *fileName) {
  // open file
  int fd;
  BF_ErrorCode code = BF_OpenFile(fileName, &fd);
  if (code != BF_OK) {
    BF_PrintError(code);
    return NULL;
  }

  // create metadata block (first block of file)
  BF_Block *metadata_block;
  BF_Block_Init(&metadata_block);
  code = BF_AllocateBlock(fd, metadata_block);
  if (code != BF_OK) {
    BF_PrintError(code);
    BF_Block_Destroy(&metadata_block);
    return NULL;
  }

  // change content of metadata block
  // sdata: starting point of data (Do not move pointer)
  // ndata: next data (Can move pointer)
  char *sdata = BF_Block_GetData(metadata_block);
  char *ndata = sdata;
  HP_info *header_info = malloc(sizeof(HP_info));
  header_info->fileDesc = fd;
  header_info->capacity = CAPACITY;
  memcpy(ndata, header_info, sizeof(HP_info));

  // move pointer to store HP_Block_info in the end of the file
  ndata += CAPACITY * sizeof(Record);
  HP_Block_info block_info;
  block_info.records = 0;
  block_info.next_block = NULL;
  memcpy(ndata, &block_info, sizeof(HP_Block_info));

  // commit changes
  BF_Block_SetDirty(metadata_block);
  code = BF_UnpinBlock(metadata_block);
  if (code != BF_OK) {
    BF_PrintError(code);
    BF_Block_Destroy(&metadata_block);
    return NULL;
  }

  BF_Block_Destroy(&metadata_block);
  return header_info;
}

int HP_CloseFile(HP_info *hp_info) {
  // close file
  CALL_BF(BF_CloseFile(hp_info->fileDesc));
  free(hp_info);
  return HP_OK;
}

int HP_InsertEntry(HP_info *hp_info, Record record) {
  // insert entry

  // get last block
  int blocks;
  CALL_BF(BF_GetBlockCounter(hp_info->fileDesc, &blocks));
  int last_id = blocks - 1;
  BF_Block *last_block;
  BF_Block_Init(&last_block);
  CALL_BF(BF_GetBlock(hp_info->fileDesc, last_id, last_block));
  char *sdata = BF_Block_GetData(last_block);
  char *ndata = sdata;

  // get HP_block_info of last block
  ndata += CAPACITY * sizeof(Record);
  HP_Block_info block_info;
  memcpy(&block_info, ndata, sizeof(HP_Block_info));

  // Check if current last block is full or if it is metadata block
  if (block_info.records == CAPACITY || last_id == 0) {
    // allocate new block
    BF_Block *new_block;
    BF_Block_Init(&new_block);
    CALL_BF(BF_AllocateBlock(hp_info->fileDesc, new_block));
    char *sdata_new = BF_Block_GetData(new_block);
    char *ndata_new = sdata_new;

    // Update next block of previous last block
    block_info.next_block = new_block;
    memcpy(ndata, &block_info, sizeof(HP_Block_info));

    // insert first entry of new_block
    memcpy(ndata_new, &record, sizeof(Record));

    // create HP_Block_info for new_block
    ndata_new += CAPACITY * sizeof(Record);
    HP_Block_info new_info;
    new_info.records = 1;
    new_info.next_block = NULL;
    memcpy(ndata_new, &new_info, sizeof(HP_Block_info));

    BF_Block_SetDirty(new_block);
    CALL_BF(BF_UnpinBlock(new_block));
    BF_Block_Destroy(&new_block);
  } else {
    // insert entry inside current block (last_block)
    ndata = sdata + block_info.records * sizeof(Record);
    memcpy(ndata, &record, sizeof(Record));
    block_info.records++;
    ndata = sdata + CAPACITY * sizeof(Record);
    memcpy(ndata, &block_info, sizeof(HP_Block_info));
    BF_Block_SetDirty(last_block);
  }
  CALL_BF(BF_UnpinBlock(last_block));
  BF_Block_Destroy(&last_block);
  return HP_OK;
}

int HP_GetAllEntries(HP_info *hp_info, int value) {
  // get all entries
  return HP_OK;
}
