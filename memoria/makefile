#GLOBALES
CC = gcc
OUTFILE = memoria.out

SRC = $(shell find ./src -type f -iname "*.c" -exec basename {} \;)
OBJ = $(SRC:%.c=./obj/%.o)

SHARED = $(shell find ../shared/obj/*.o)

build: makefolder makeshared $(OBJ)
	$(CC) -o $(OUTFILE) main.c $(OBJ) $(SHARED) $(LINK)
	
LINK = -lpthread -lcommons -lm -lreadline 
CFLAGS = -c

debug: CFLAGS := $(CFLAGS) -g
debug: makeshared_debug build

./obj/%.o: ./src/%.c
	$(CC) $(CFLAGS) $< -o $@ $(LINK)

makefolder:
	if [ ! -d ./obj ]; then mkdir obj; fi

makeshared:
	make -C ../shared

makeshared_debug:
	make debug -C ../shared
	
#CLEAN

clean:
	rm -rf ./obj
	rm -rf *.out
	make clean -C ../shared
	