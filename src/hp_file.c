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

int HP_CreateFile(char *fileName) {
  // create file
  return HP_OK;
}

HP_info *HP_OpenFile(char *fileName) {
  // open file
  return NULL;
}

int HP_CloseFile(HP_info *hp_info) {
  // close file
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
