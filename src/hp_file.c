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
  // open file
  int fd;
  CALL_BF(BF_OpenFile(fileName, &fd));
  // create metadata block (first block of file)
  BF_Block *metadata_block;
  BF_Block_Init(&metadata_block);
  CALL_BF(BF_AllocateBlock(fd, metadata_block));

  // change content of metadata block
  // sdata: starting point of data (Do not move pointer)
  // ndata: next data (Can move pointer)
  char *sdata = BF_Block_GetData(metadata_block);
  char *ndata = sdata;
  HP_info hp_info;
  memset(&hp_info, 0, sizeof(HP_info));
  strncpy(hp_info.type, "Heap_File", 10 * sizeof(char));
  hp_info.fileDesc = fd;
  hp_info.capacity = CAPACITY;
  memcpy(ndata, &hp_info, sizeof(HP_info));

  // move pointer to store HP_Block_info in the end of the file
  ndata += CAPACITY * sizeof(Record);
  HP_Block_info block_info;
  block_info.records = 0;
  memcpy(ndata, &block_info, sizeof(HP_Block_info));

  // commit changes
  BF_Block_SetDirty(metadata_block);
  CALL_BF(BF_UnpinBlock(metadata_block));

  BF_Block_Destroy(&metadata_block);
  CALL_BF(BF_CloseFile(fd));
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

  // read meatadata block (first block)
  BF_Block *metadata_block;
  BF_Block_Init(&metadata_block);
  code = BF_GetBlock(fd, 0, metadata_block);
  if (code != BF_OK) {
    BF_PrintError(code);
    BF_Block_Destroy(&metadata_block);
    return NULL;
  }

  // read HP_info from metadata block
  char *sdata = BF_Block_GetData(metadata_block);
  char *ndata = sdata;
  HP_info *hp_info = malloc(sizeof(HP_info));
  memcpy(hp_info, ndata, sizeof(HP_info));

  // check if file is a heap file
  if (strcmp(hp_info->type, "Heap_File")) {
    printf("Given file is not of type Heap_file\n");
    return NULL;
  }

  code = BF_UnpinBlock(metadata_block);
  if (code != BF_OK) {
    BF_PrintError(code);
    BF_Block_Destroy(&metadata_block);
    return NULL;
  }

  BF_Block_Destroy(&metadata_block);
  return hp_info;
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

    // insert first entry of new_block
    memcpy(ndata_new, &record, sizeof(Record));

    // create HP_Block_info for new_block
    ndata_new += CAPACITY * sizeof(Record);
    HP_Block_info new_info;
    new_info.records = 1;
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

  int searched_blocks = 0;

  // get number of blocks
  int blocks;
  CALL_BF(BF_GetBlockCounter(hp_info->fileDesc, &blocks));

  // sequential search in all blocks
  for (int i = 1; i < blocks; i++) {
    BF_Block *curr_block;
    BF_Block_Init(&curr_block);
    CALL_BF(BF_GetBlock(hp_info->fileDesc, i, curr_block));
    char *sdata = BF_Block_GetData(curr_block);
    char *ndata = sdata;

    ndata += CAPACITY * sizeof(Record);
    HP_Block_info block_info;
    memcpy(&block_info, ndata, sizeof(HP_Block_info));

    // search all records inside each block
    for (int j = 0; j < block_info.records; j++) {
      Record record;
      ndata = sdata + j * sizeof(Record);
      memcpy(&record, ndata, sizeof(Record));
      if (record.id == value) {
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
        return searched_blocks;
      }
    }

    CALL_BF(BF_UnpinBlock(curr_block));
    BF_Block_Destroy(&curr_block);
    searched_blocks++;
  }

  return searched_blocks;
}
