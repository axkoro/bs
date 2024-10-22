#!/usr/bin/make
.SUFFIXES:
SRC = $(wildcard *.c) $(wildcard *.s)
HDR = $(wildcard *.h)
BIN = bootloader.bin
PCK = lab-1.zip

CFLAGS = -m32 -c -Os -march=i686 -ffreestanding -Wall
LFLAGS = -m elf_i386 -static -nostdlib --nmagic
AFLAGS =

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.s
	$(AS) $(AFLAGS) -o $@ $^

%.elf: %.o %.ld
	$(LD) $(LFLAGS) -T$*.ld -o $@ $<

%.bin: %.elf
	objcopy -O binary $^ $@

kernel: $(BIN)
	cat $^ > $@

all: kernel	

run: all
	qemu-system-x86_64 -drive format=raw,file=kernel -nographic

pack:
	zip $(PCK) Makefile $(SRC) $(HDR) $(BIN:%.bin=%.ld)

clean:
	$(RM) $(RMFILES) $(BIN) $(PCK) kernel
