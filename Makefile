### Default target: compile all examples linked with source files
all: hp bf ht sht main stat

all-hp: clean-db hp run-hp

all-ht: clean-db ht run-ht

all-sht: clean-db sht run-sht

all-main: clean-db main run-main

all-stat: stat run-stat

Build = ./build
Include = ./include
Lib = ./lib
Examples = ./examples
SRC = ./src

### Normal targets

hp:
	@echo " Compile hp_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/hp_main.c $(SRC)/record.c $(SRC)/hp_file.c -lbf -o $(Build)/hp_main -O2

bf:
	@echo " Compile bf_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/bf_main.c $(SRC)/record.c -lbf -o $(Build)/bf_main -O2

ht:
	@echo " Compile ht_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/ht_main.c $(SRC)/record.c $(SRC)/ht_table.c -lbf -o $(Build)/ht_main -O2

sht:
	@echo " Compile sht_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/sht_main.c $(SRC)/record.c $(SRC)/sht_table.c $(SRC)/ht_table.c -lbf -o $(Build)/sht_main -O2

main:
	@echo " Compile all_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/all_main.c $(SRC)/record.c $(SRC)/sht_table.c $(SRC)/ht_table.c $(SRC)/hp_file.c -lbf -o $(Build)/all_main -O2

stat:
	@echo " Compile stat_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/stat_main.c $(SRC)/record.c $(SRC)/sht_table.c $(SRC)/ht_table.c -lbf -o $(Build)/stat_main -O2


run-hp: clean-db
	$(Build)/hp_main

run-ht: clean-db
	$(Build)/ht_main

run-sht: clean-db
	$(Build)/sht_main

run-main: clean-db
	$(Build)/all_main

run-stat:
	$(Build)/stat_main

Exec = $(Build)/*

clean:
	rm -f $(Exec)
	find . -name \*.db -type f -delete

clean-exec:
	rm -f $(Exec)

clean-db:
	find . -name \*.db -type f -delete

### Debugging targets

Debug_Flags = -g3 -DDEBUG -Wall

hp-deb:
	@echo " Compile hp_main with debug flags instead of optimization flag ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/hp_main.c $(SRC)/record.c $(SRC)/hp_file.c -lbf -o $(Build)/hp_main $(Debug_Flags)

ht-deb:
	@echo " Compile hp_main with debug flags instead of optimization flag ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/ht_main.c $(SRC)/record.c $(SRC)/ht_table.c -lbf -o $(Build)/ht_main $(Debug_Flags)

sht-deb:
	@echo " Compile hp_main with debug flags instead of optimization flag ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/sht_main.c $(SRC)/record.c $(SRC)/sht_table.c $(SRC)/ht_table.c -lbf -o $(Build)/sht_main $(Debug_Flags)

deb: hp-deb ht-deb sht-deb

gdb-hp: hp-deb
	gdb ./$(Build)/hp_main

gdb-ht: ht-deb
	gdb ./$(Build)/ht_main

gdb-sht: sht-deb
	gdb ./$(Build)/sht_main

valgrind-hp: clean hp-deb
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(Build)/hp_main

valgrind-ht: clean ht-deb
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(Build)/ht_main

valgrind-sht: clean sht-deb
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(Build)/sht_main