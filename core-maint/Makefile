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

all: debug2

debug: CC+= -ggdb
debug2: CC+= -ggdb
core: CC+= -O3
core2: CC+= -O3

core: main.o gadget.o heap.o treap.o traversal.o glist.o ours.o ./gm_graph/lib/libgmgraph.a
	$(CC) $(CFLAGS2) main.o gadget.o heap.o treap.o traversal.o glist.o ours.o  $(LFLAGS) -o core

debug: main.o gadget.o heap.o treap.o traversal.o glist.o ours.o ./gm_graph/lib/libgmgraph.a
	$(CC) $(CFLAGS2) main.o gadget.o heap.o treap.o traversal.o glist.o ours.o  $(LFLAGS) -o debug

core2: main.o gadget.o heap.o treap.o traversal.o glist.o ours.o ourspar.o om.o ./gm_graph/lib/libgmgraph.a
	$(CC) $(CFLAGS2) main.o gadget.o heap.o treap.o traversal.o glist.o ours.o ourspar.o om.o $(LFLAGS) -o core2

debug2: main.o gadget.o heap.o treap.o traversal.o glist.o ours.o ourspar.o om.o ./gm_graph/lib/libgmgraph.a
	$(CC) $(CFLAGS2) main.o gadget.o heap.o treap.o traversal.o glist.o ours.o ourspar.o om.o $(LFLAGS) -o debug2


main.o: main.cc
	$(CC) $(CFLAGS) main.cc -o main.o


gadget.o: gadget/gadget.cc
	$(CC) $(CFLAGS) gadget/gadget.cc -o gadget.o

heap.o: gadget/heap.cc
	$(CC) $(CFLAGS) gadget/heap.cc -o heap.o

treap.o: gadget/treap.cc
	$(CC) $(CFLAGS) gadget/treap.cc -o treap.o

traversal.o: traversal/traversal.cc
	$(CC) $(CFLAGS) traversal/traversal.cc -o traversal.o

glist.o: glist/glist.cc
	$(CC) $(CFLAGS) glist/glist.cc -o glist.o

# ours-seq.o: ours-seq/core-maint.cpp
# 	$(CC) $(CFLAGS) ours-seq/core-maint.cpp -o ours-seq.o

#ours-seq-csr.o: ours-seq-csr/core-maint-csr.cpp
#	$(CC) $(CFLAGS) ours-seq-csr/core-maint-csr.cpp -o ours-seq-csr.o

ours.o: ours-csr-new/seq-csr-new.cpp
	$(CC) $(CFLAGS) ours-csr-new/seq-csr-new.cpp -o ours.o

ourspar.o: ours-csr-new/par-csr-new.cpp
	$(CC) $(CFLAGS) ours-csr-new/par-csr-new.cpp -o ourspar.o

om.o: ours-csr-new/par-om.cpp
	$(CC) $(CFLAGS) ours-csr-new/par-om.cpp -o om.o

clean2: 
	rm -f ./ourspar.o ./om.o
clean: 
	rm -f ./*.o

