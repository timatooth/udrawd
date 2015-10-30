CXX=g++
CFLAGS=-c -std=c++11 
LIBS=-lboost_system -lssl -lcrypto -lnghttp2_asio -lhiredis 
UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
	# mac homebrew explicit for now
	CFLAGS := $(CFLAGS) \
		-I/usr/local/Cellar/hiredis/0.13.3/include \
		-I/usr/local/Cellar/nghttp2/1.4.0/include \
		-I/usr/local/Cellar/boost/1.58.0/include \
		-I/usr/local/opt/openssl/include
	LIBS := $(LIBS) \
		-L/usr/local/Cellar/hiredis/0.13.3/lib \
		-L/usr/local/Cellar/nghttp2/1.4.0/lib \
		-L/usr/local/Cellar/boost/1.58.0/lib \
		-L/usr/local/opt/openssl/lib
endif

udrawd: udrawd.o
	$(CXX) udrawd.o -o udrawd $(LIBS)

clean:
	rm *.o udrawd

udrawd.o: udrawd.cpp
	$(CXX) udrawd.cpp $(CFLAGS)

all: udrawd
