#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

int HP_CreateFile(char *fileName, int attr){
    return -1;
}

HP_info* HP_OpenFile(char *fileName){
    return malloc(sizeof(HP_info));

}


int HP_CloseFile( HP_info* header_info ){
    return -1;
}

int HP_InsertEntry(HP_info* header_info, Record record){
        return -1;
}

int HP_GetAllEntries(HP_info* header_info, void *value ){
    return -1;
}

