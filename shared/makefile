#GLOBALES
CC = gcc

SRC = $(shell find ./src -type f -iname "*.c" -exec basename {} \;)
OBJ = $(SRC:%.c=./obj/%.o)


build: makefolder $(OBJ)

LINK = -lpthread -lcommons -lm -lreadline
CFLAGS = -c -fpic

debug: CFLAGS := $(CFLAGS) -g
debug: build

./obj/%.o: ./src/%.c
	$(CC) $(CFLAGS) $< -o $@ $(LINK)

makefolder:
	if [ ! -d ./obj ]; then mkdir obj; fi

#CLEAN

clean:
	rm -rf ./obj
	rm -rf *.out

