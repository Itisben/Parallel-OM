CC=g++

INCDIR=./gm_graph/inc
LIBDIR=./gm_graph/lib
LIB=gmgraph
LFLAGS=-L$(LIBDIR) -l$(LIB)


CFLAGS=-c -I. -I$(INCDIR) -fopenmp  -std=gnu++0x -fpermissive -Wfatal-errors -w
CFLAGS2=-I. -I$(INCDIR) -fopenmp  -std=gnu++0x -fpermissive -Wfatal-errors -w
INCFILES = $(shell ls $(INCDIR))
H_FILES = $(addprefix $(INCDIR)/, $(INCFILES))

OUTDIR=./

all: core

debug: CC+= -ggdb
core: CC+= -O3


core: main.o om.o 
	$(CC) $(CFLAGS2) main.o om.o -o core

debug: main.o om.o
	$(CC) $(CFLAGS2) main.o om.o -o debug


main.o: main.cc
	$(CC) $(CFLAGS) main.cc -o main.o


om.o: ours-csr-new/par-om.cpp
	$(CC) $(CFLAGS) ours-csr-new/par-om.cpp -o om.o

clean: 
	rm -f ./*.o

