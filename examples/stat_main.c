#include "bf.h"
#include "ht_table.h"
#include "sht_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HT_FILENAME "hash.db"
#define SHT_FILENAME "secondary_index.db"

#define ASSERT(ret_value)                                                      \
  {                                                                            \
    if (ret_value < 0) {                                                       \
      printf("\nFunction failed. Aborting\n");                                 \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  }

#define CALL_BF(call)                                                          \
  {                                                                            \
    BF_ErrorCode code = call;                                                  \
    if (code != BF_OK) {                                                       \
      BF_PrintError(code);                                                     \
      exit(code);                                                              \
    }                                                                          \
  }

int main(void) {
  CALL_BF(BF_Init(LRU));
  printf("Print Statistics of Hash file with primary index\n");
  ASSERT(HT_HashStatistics(HT_FILENAME));
  printf("\n");
  printf("Print statistics of secondary index\n");
  ASSERT(SHT_HashStatistics(SHT_FILENAME));
  CALL_BF(BF_Close());
  return 0;
}