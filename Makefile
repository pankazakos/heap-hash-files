### Default target: compile all examples linked with source files
compile: all

all: hp bf ht

all-hp: clean run-hp

all-ht: clean run-ht

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
	@echo " Compile hp_main ...";
	gcc -I $(Include) -L $(Lib) -Wl,-rpath,$(Lib) $(Examples)/ht_main.c $(SRC)/record.c $(SRC)/ht_table.c -lbf -o $(Bin)/ht_main -O2

run-hp: hp
	./$(Bin)/hp_main

run-ht: ht
	./$(Bin)/ht_main

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

deb: hp-deb ht-deb

gdb-hp: hp-deb
	gdb ./$(Bin)/hp_main

gdb-ht: ht-deb
	gdb ./$(Bin)/ht_main

valgrind-hp: clean hp-deb
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(Bin)/hp_main

valgrind-ht: clean ht-deb
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(Bin)/ht_main