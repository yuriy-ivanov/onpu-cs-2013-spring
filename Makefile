CC=g++
RM=rm -rf
CFLAGS=-g
TARGET=small
UNAME=$(shell uname)

ifeq ($(UNAME), Darwin)
IFLAGS=-I /opt/local/include
else
IFLAGS=-I /path/to/my/includes/where/boost/lives
endif


all: main.cpp
	@echo "OS: $(UNAME)"
	$(CC) $(CFLAGS) $(IFLAGS) -o $(TARGET) main.cpp

clean:
	$(RM) small small.dSYM/
