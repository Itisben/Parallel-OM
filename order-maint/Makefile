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


core: main.o om.o om2.o
	$(CC) $(CFLAGS2) main.o om.o om2.o -o core

debug: main.o om.o om2.o
	$(CC) $(CFLAGS2) main.o om.o om2.o -o debug


main.o: main.cc
	$(CC) $(CFLAGS) main.cc -o main.o


om.o: ours/par-om.cpp
	$(CC) $(CFLAGS) ours/par-om.cpp -o om.o

om2.o: ours/par-om2.cpp
	$(CC) $(CFLAGS) ours/par-om2.cpp -o om2.o

clean: 
	rm -f ./*.o

