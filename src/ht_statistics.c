#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "ht_main.c"
#include "ht_table.c"

#define RECORDS_NUM 200 // you can change it if you want

int HashStatistics(char *filename) {
  HT_block_info block_info;
  /*For counting blocks*/
  int blockIds[200];
  int blockIdsLen;
  int num_blocks;

  /*For buckets*/
  int minrec = 0;
  int avgrec = 0;
  int maxrec = 0;

  HT_CreateFile(filename, 10);
  HT_info *info = HT_OpenFile(filename);

  Record record;
  srand(12569874);
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();

    // change the elements of blockIds
    blockId = HT_InsertEntry(info, record);
    blockIds[blockId]++;

    // Just an idea
    if (minrec > block_info.records) {
      minrec = block_info.records;
    }
    elseif(maxrec < block_info.records) { maxrec = block_info.records; }
  }
}

// If an element of blockIds != 0 it's a block
blockIdsLen = sizeof(blockIds) / sizeof(blockIds[0]);
for (int i = 0; i < blockIdLen; ++i) {
  if (blockIds[i] != 0) {
    num_blocks++;
  }
}

for (int id = 0; id < RECORDS_NUM; ++id) {
  HT_GetAllEntries(info, &id);
}
}