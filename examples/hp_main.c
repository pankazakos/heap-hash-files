#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bf.h"
#include "hp_file.h"

#define RECORDS_NUM 1000 // you can change it if you want
#define FILE_NAME "heap.db"

#define CALL_OR_DIE(call)                                                      \
  {                                                                            \
    BF_ErrorCode code = call;                                                  \
    if (code != BF_OK) {                                                       \
      BF_PrintError(code);                                                     \
      exit(code);                                                              \
    }                                                                          \
  }

// call functions only in debug mode. Usage d(func())
#ifdef DEBUG
#define d(x) x
#else
#define d(x)
#endif

int main() {
  BF_Init(LRU);

  HP_CreateFile(FILE_NAME);
  HP_info *info = HP_OpenFile(FILE_NAME);

  Record record;
  srand(12569874);
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    HP_InsertEntry(info, record);
  }

  printf("RUN PrintAllEntries\n");
  int id = rand() % RECORDS_NUM;
  printf("\nSearching for: %d\n", id);
  HP_GetAllEntries(info, id);

  HP_CloseFile(info);
  BF_Close();
}
