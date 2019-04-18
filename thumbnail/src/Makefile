# vim: noexpandtab

CPPFLAGS = -std=c++14
CPPFLAGS += $(shell PKG_CONFIG_PATH=/home/gudok/SVN/3rdparty/opencv/lib/pkgconfig pkg-config --static --cflags opencv)

LIBS = 
LIBS += $(shell PKG_CONFIG_PATH=/home/gudok/SVN/3rdparty/opencv/lib/pkgconfig pkg-config --static --libs opencv)
LIBS += -lzlib

EXECUTABLES = gen

all: $(EXECUTABLES)

$(EXECUTABLES): %: %.o
	g++ $< $(LIBS) -o $@

%.o: %.cpp
	g++ $(CPPFLAGS) $< -c -o $@

clean:
	rm -f $(EXECUTABLES) $(addsuffix .o, $(EXECUTABLES))

.PHONY: all clean

