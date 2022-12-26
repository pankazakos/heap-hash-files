#include "bf.h"
#include "hp_file.h"
#include "ht_table.h"
#include "sht_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RECORDS_NUM 300
#define HP_FILENAME "heap.db"
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

  srand(time(NULL));

  // Create files
  CALL_BF(BF_Init(LRU));
  ASSERT(HP_CreateFile(HP_FILENAME));
  int buckets = 20;
  ASSERT(HT_CreateFile(HT_FILENAME, buckets));
  ASSERT(SHT_CreateSecondaryIndex(SHT_FILENAME, buckets, HT_FILENAME));
  CALL_BF(BF_Close());

  //
  /////////////////////////////////////////////////////////
  // Heap file

  printf("\033[1mHeap_File\033[m\n");
  CALL_BF(BF_Init(LRU));
  HP_info *hp_info = HP_OpenFile(HP_FILENAME); // Open file
  if (hp_info == NULL) {
    printf("hp_info is NULL. Aborting\n");
    exit(EXIT_FAILURE);
  }

  // Insert records
  printf("Inserting records in heap file... ");
  for (int i = 0; i < RECORDS_NUM; i++) {
    Record record = randomRecord();
    ASSERT(HP_InsertEntry(hp_info, record));
  }
  printf("\033[1;32mOK\033[m\n"); // prints green OK

  // Search one specific record with primary key in heap file
  int id = rand() % RECORDS_NUM; // id range: [0, RECORDS_NUM)
  printf("\033[1;36mSearching for id\033[m: %d\n", id);
  ASSERT(HP_GetAllEntries(hp_info, id));
  ASSERT(HP_CloseFile(hp_info)); // Close heap file
  CALL_BF(BF_Close());

  printf("\n\n");

  //
  /////////////////////////////////////////////////////////
  // Hash file with primary and secondary indexes

  printf("\033[1mHash_File and Secondary Index\033[m\n");
  CALL_BF(BF_Init(LRU));
  HT_info *ht_info = HT_OpenFile(HT_FILENAME); // Open file
  if (ht_info == NULL) {
    printf("ht_info is NULL. Aborting\n");
    exit(EXIT_FAILURE);
  }
  SHT_info *sht_info = SHT_OpenSecondaryIndex(SHT_FILENAME); // Open file
  if (sht_info == NULL) {
    printf("sht_info is NULL. Aborting\n");
    exit(EXIT_FAILURE);
  }

  // Insert records
  printf("Inserting records in hash file... ");
  for (int i = 0; i < RECORDS_NUM; i++) {
    Record record = randomRecord();
    int block_id = HT_InsertEntry(ht_info, record);
    ASSERT(block_id);
    ASSERT(SHT_SecondaryInsertEntry(sht_info, record, block_id));
  }
  printf("\033[1;32mOK\033[m\n"); // prints green OK

  // Search one specific record with primary key in hash file
  id = rand() % (2 * RECORDS_NUM - RECORDS_NUM + 1) +
       RECORDS_NUM; // id range: [RECORDS_NUM, 2*RECORDS_NUM)
  printf("\033[1;36mSearching for id\033[m: %d\n", id);
  ASSERT(HT_GetAllEntries(ht_info, &id));

  // Print all records containing name
  Record record = randomRecord();
  printf("\nPrint all entries for name: %s\n", record.name);
  ASSERT(SHT_SecondaryGetAllEntries(ht_info, sht_info, record.name))
  ASSERT(HT_CloseFile(ht_info));             // Close hash file
  ASSERT(SHT_CloseSecondaryIndex(sht_info)); // Close secondary index
  CALL_BF(BF_Close());

  return 0;
}