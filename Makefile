### Default target: compile all examples linked with source files
compile: all

all: hp bf ht

all-hp: clean run-hp

all-ht: clean run-ht

all-main: clean main
	./build/all_main

all-stat: stat
	./build/stat_main

Bin = ./build
Include = ./include
Lib = ./lib
Examples = ./examples
SRC = ./src

### Normal targets

hp:
	@echo " Compile hp_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/hp_main.c $(SRC)/record.c $(SRC)/hp_file.c -lbf -o $(Bin)/hp_main -O2

bf:
	@echo " Compile bf_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/bf_main.c $(SRC)/record.c -lbf -o $(Bin)/bf_main -O2

ht:
	@echo " Compile ht_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/ht_main.c $(SRC)/record.c $(SRC)/ht_table.c -lbf -o $(Bin)/ht_main -O2

sht:
	@echo " Compile sht_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/sht_main.c $(SRC)/record.c $(SRC)/sht_table.c $(SRC)/ht_table.c -lbf -o $(Bin)/sht_main -O2

main:
	@echo " Compile all_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/all_main.c $(SRC)/record.c $(SRC)/sht_table.c $(SRC)/ht_table.c $(SRC)/hp_file.c -lbf -o $(Bin)/all_main -O2

stat:
	@echo " Compile stat_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/stat_main.c $(SRC)/record.c $(SRC)/sht_table.c $(SRC)/ht_table.c -lbf -o $(Bin)/stat_main -O2


run-hp: hp
	./$(Bin)/hp_main

run-ht: ht
	./$(Bin)/ht_main

run-sht:
	./$(Bin)/sht_main

Exec = $(Bin)/*

clean:
	rm -f $(Exec)
	find . -name \*.db -type f -delete

### Debugging targets

Debug_Flags = -g3 -DDEBUG -Wall

hp-deb:
	@echo " Compile hp_main with debug flags instead of optimization flag ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/hp_main.c $(SRC)/record.c $(SRC)/hp_file.c -lbf -o $(Bin)/hp_main $(Debug_Flags)

ht-deb:
	@echo " Compile hp_main with debug flags instead of optimization flag ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/ht_main.c $(SRC)/record.c $(SRC)/ht_table.c -lbf -o $(Bin)/ht_main $(Debug_Flags)

sht-deb:
	@echo " Compile hp_main with debug flags instead of optimization flag ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/sht_main.c $(SRC)/record.c $(SRC)/sht_table.c $(SRC)/ht_table.c -lbf -o $(Bin)/sht_main $(Debug_Flags)

deb: hp-deb ht-deb sht-deb

gdb-hp: hp-deb
	gdb ./$(Bin)/hp_main

gdb-ht: ht-deb
	gdb ./$(Bin)/ht_main

gdb-sht:
	gdb ./$(Bin)/sht_main

valgrind-hp: clean hp-deb
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(Bin)/hp_main

valgrind-ht: clean ht-deb
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(Bin)/ht_main

valgrind-sht: clean sht-deb
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(Bin)/sht_main