CC=g++
RM=rm -rf
IFLAGS=-I /opt/local/include
CFLAGS=-g
TARGET=small

all: main.cpp
	$(CC) $(CFLAGS) $(IFLAGS) -o $(TARGET) main.cpp

clean:
	$(RM) small small.dSYM/
