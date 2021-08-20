# vim: noexpandtab

OPTS=--std=c++17 -O3 -g -march=native -mtune=native -Wall
#COMPILER=clang++-11
COMPILER=g++-10

transpose: transpose.cc *.h
	$(COMPILER) transpose.cc $(OPTS) -o transpose

transpose.s: transpose.cc *.h
	$(COMPILER) transpose.cc $(OPTS) -o transpose.s -S

mca:
	$(COMPILER) transpose.cc $(OPTS) -S
	llvm-mca-9 --iterations=1 --all-views < transpose.s > mca.out


all:
	./codegen.py
	$(COMPILER) transpose.cc $(OPTS) -o transpose
	$(COMPILER) transpose.cc $(OPTS) -S
	llvm-mca-9 --iterations=1 --all-views < transpose.s > mca.out
	grep -A 8 'Code Region' mca.out

clean:
	rm -f transpose transpose.s transpose.o

.PHONY: all clean

