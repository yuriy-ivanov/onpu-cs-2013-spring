CC=g++
RM=rm -rf
CFLAGS=-g
TARGET=small
ROOT_SRC_DIR=./src/
ROOT_BIN_DIR=./bin/
UNAME=$(shell uname)

ifeq ($(UNAME), Darwin)
IFLAGS=-I /opt/local/include
else
IFLAGS=
endif


all: $(ROOT_SRC_DIR)main.cpp
	@echo "OS: $(UNAME)"
	@$(CC) $(CFLAGS) $(IFLAGS) -o $(ROOT_BIN_DIR)$(TARGET) $(ROOT_SRC_DIR)main.cpp

clean:
	@$(RM) $(ROOT_BIN_DIR)$(TARGET) $(ROOT_BIN_DIR)$(TARGET).dSYM/
