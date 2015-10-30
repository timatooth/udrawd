CC=g++
CFLAGS=-c -std=gnu++11 -fpermissive -g
LIBS=-lboost_system -lssl -lcrypto -lnghttp2_asio -lhiredis -g

udrawd: udrawd.o
	$(CC) udrawd.o -o udrawd $(LIBS)

clean:
	rm *.o udrawd

udrawd.o: udrawd.cpp
	$(CC) udrawd.cpp $(CFLAGS)

all: udrawd
