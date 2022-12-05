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
  char *data = BF_Block_GetData(metadata_block);
  HP_info *header_info = malloc(sizeof(HP_info));
  header_info->fileDesc = fd;
  header_info->capacity = CAPACITY;
  memcpy(data, header_info, sizeof(HP_info));
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
  return HP_OK;
}

int HP_GetAllEntries(HP_info *hp_info, int value) {
  // get all entries
  return HP_OK;
}
