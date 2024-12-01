#!/usr/bin/make
.SUFFIXES:
.PHONY: all run pack clean
.SILENT: run

TAR = libshared.a client server
PCK = lab-3.zip
LIB = libshared.a

CFLAGS = -std=gnu11 -c -g -Os -Wall -MMD -MP -I "lib"
LFLAGS = -pthread
AFLAGS = rcs

SHARED_SRC = $(wildcard lib/*.c)
SHARED_OBJ = $(SHARED_SRC:%.c=%.o)
SHARED_DEP = $(SHARED_SRC:%.c=%.d)

CLIENT_SRC = $(wildcard cli/*.c)
CLIENT_OBJ = $(CLIENT_SRC:%.c=%.o)
CLIENT_DEP = $(CLIENT_SRC:%.c=%.d)

SERVER_SRC = $(wildcard srv/*.c)
SERVER_OBJ = $(SERVER_SRC:%.c=%.o)
SERVER_DEP = $(SERVER_SRC:%.c=%.d)

-include $(SHARED_DEP) $(CLIENT_DEP) $(SERVER_DEP)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

client: $(CLIENT_OBJ) $(LIB)
	$(CC) -o $@ $^ $(LFLAGS)

server: $(SERVER_OBJ) $(LIB)
	@if [ -e srv_pid.txt ]; then kill `cat srv_pid.txt`; $(RM) $(RMFILES) srv_pid.txt; fi
	$(CC) -o $@ $^ $(LFLAGS)

libshared.a: $(SHARED_OBJ)
	$(AR) $(AFLAGS) $@ $^

all: $(TAR)

srv_pid.txt: server
	./server& echo $$! > $@

run: all srv_pid.txt
	./client

pack: clean
	zip -r $(PCK) cli srv lib Makefile -x "*/.*"

clean:
	@[ -e srv_pid.txt ] && kill `cat srv_pid.txt` || true
	$(RM) $(RMFILES) srv_pid.txt $(TAR) $(PCK) $(CLIENT_OBJ) $(CLIENT_DEP) $(SERVER_OBJ) $(SERVER_DEP) $(SHARED_OBJ) $(SHARED_DEP)
