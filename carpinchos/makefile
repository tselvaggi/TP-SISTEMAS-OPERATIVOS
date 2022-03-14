#GLOBALES
CC = gcc
OUTFILE = carpinchos.out

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

#MATELIB

mate-build: makefolderlog
	$(CC) -c -Wall -Werror -I"../../" -fpic src/matelib.c && \
	$(CC) -shared $(OBJ) $(SHARED) -o libmatelib.so && \
	rm matelib.o
	sudo cp ./libmatelib.so /lib
	sudo cp ./libmatelib.so /usr/lib
	cp ./carpinchos_kernel.config /home/utnso/carpinchos-pruebas/build/
	cp ./carpinchos_memoria.config /home/utnso/carpinchos-pruebas/build/
	
build-example: build-example-lib
	cd examples/lib_usage && \
	$(CC) -g3 -Wall -L"../lib_implementation" -I"../../" main.c -lmate -lpthread -lrt -o example

run: build-example
	cd examples/lib_usage && \
	LD_LIBRARY_PATH="../lib_implementation/" ./example
	
makefolderlog:
	if [ ! -d /home/utnso/carpinchos-pruebas/build/log ]; then mkdir /home/utnso/carpinchos-pruebas/build/log; fi

clean:
	rm examples/lib_implementation/libmate.so examples/lib_usage/example

#CLEAN

clean:
	rm -rf ./obj
	rm -rf *.out
	make clean -C ../shared