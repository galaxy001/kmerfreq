CC = gcc
CXX = g++
CFLAGS = -std=gnu11 -Wall -pedantic -pipe -march=core2 -mtune=generic -I/usr/local/include
CXXFLAGS = -std=gnu++14 -Wall -pedantic -pipe -march=core2 -mtune=generic -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lz #-lhts
OPT = -O2

exefiles = kmerfreq

CFLAGS += -pthread -D_REENTRANT -D_THREAD_SAFE
CFLAGS += -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wshadow -Wpointer-arith -Wsign-compare -Wcast-qual

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	#CC = gcc-9
	#CXX = g++-9
	LDFLAGS += #-largp #-D_THREAD_SAFE -static-libgcc -static-libstdc++
endif
ifeq ($(UNAME_S),Linux)
	LDFLAGS += #-static -lbz2 -lm -lpthread -lz -llzma -pthread
	# https://github.com/samtools/htslib/issues/259 , thus no static !
endif

OBJDIR = ./objects/

#MAKEARG = $(CFLAGS) $(OPT)

cobjects = ./objects/seqKmer.o ./objects/seqRead.o ./objects/lfqueue.o
c99objects = 
mainobjects = ./objects/main.o
objects = $(cobjects) $(c99objects)

all: $(objects) $(exefiles)

$(exefiles): $(objects) $(cxxobjects) $(mainobjects)
	#$(CXX) $(CXXFLAGS) $(OPT) $(LDFLAGS) -o $(exefiles) $(mainobjects) $(objects)
	$(CC) $(CFLAGS) $(OPT) $(LDFLAGS) -o $(exefiles) $(mainobjects) $(objects)

$(mainobjects): $(OBJDIR)%.o: %.c tmpdir
	#$(CXX) $(CXXFLAGS) $(OPT) -c $< -o $@
	$(CC) $(CFLAGS) $(OPT) -c $< -o $@

$(cobjects): $(OBJDIR)%.o: %.c tmpdir
	$(CC) $(CFLAGS) $(OPT) -c $< -o $@

$(cxxobjects): $(OBJDIR)%.o: %.c tmpdir
	$(CXX) $(CXXFLAGS) $(OPT) -c $< -o $@

$(c99objects): $(OBJDIR)%.o: %.c tmpdir
	$(CC) $(CFLAGS) $(OPT) -c $< -o $@

debug: override OPT := -O -D DEBUG -g
debug: all

tmpdir:
	-mkdir ./objects

test: override OPT := -O -D DEBUG -g
ValgridnOPT := --leak-check=full --leak-resolution=med --track-origins=yes --vgdb=no
test: $(exefiles)
	valgrind $(ValgridnOPT) ./$(exefiles) -k 17 -t 10 -p test0 reads_files.list
run: $(exefiles)
	valgrind $(ValgridnOPT) ./$(exefiles) -k 17 -t 10 -p test0 reads_files.list

pp:
	$(CC) -E main.c | indent > mm.c
	-mate mm.c 
sign: $(exefiles)
	codesign -s "Mac Developer" -v $(exefiles)
	codesign -d -v $(exefiles)

.PHONY : clean
clean:
	-rm $(exefiles) $(mainobjects) $(objects)
	@mkdir -p $(OBJDIR)
