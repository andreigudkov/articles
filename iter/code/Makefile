EXECUTABLES = iterate

all: $(EXECUTABLES)

$(EXECUTABLES): %: %.cc
	g++ $< -o $@ -std=c++17 -I. -O3 -ggdb -Wall -Wextra

