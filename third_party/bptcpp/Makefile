CC=clang++
ARGS=-std=c++11 -stdlib=libc++

all : bpt

bpt : BPlusTree.o Exceptions.o InternalNode.o LeafNode.o Node.o Printer.o Record.o main.o
	$(CC) $(ARGS) $^ -o $@
	rm *.o

BPlusTree.o	: BPlusTree.cpp BPlusTree.hpp Definitions.hpp
	$(CC) $(ARGS) -c $<

Exceptions.o : Exceptions.cpp Exceptions.hpp Definitions.hpp
	$(CC) $(ARGS) -c $<

InternalNode.o : InternalNode.cpp InternalNode.hpp Definitions.hpp
	$(CC) $(ARGS) -c $<

LeafNode.o : LeafNode.cpp LeafNode.hpp Definitions.hpp
	$(CC) $(ARGS) -c $<

Node.o : Node.cpp Node.hpp Definitions.hpp
	$(CC) $(ARGS) -c $<

Printer.o : Printer.cpp Printer.hpp Definitions.hpp
	$(CC) $(ARGS) -c $<

Record.o : Record.cpp Record.hpp Definitions.hpp
	$(CC) $(ARGS) -c $<

main.o : main.cpp Definitions.hpp 
	$(CC) $(ARGS) -c $<

clean :
	rm -f *.o bpt
