# Simple Makefile for GRUB Multiboot kernel

SRC_ASM = src/boot.asm
SRC_C   = src/kernel.c
GRUB_CFG = src/grub.cfg
BIN     = build/kernel.bin
ISO     = build/kernel.iso

ASM     = nasm
CC      = clang
LD      = ld
CFLAGS  = -m32 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

all: $(ISO)

build:
	mkdir -p build

$(BIN): $(SRC_ASM) $(SRC_C) | build
	$(ASM) -f elf32 $(SRC_ASM) -o build/boot.o
	$(CC) $(CFLAGS) -c $(SRC_C) -o build/kernel.o
	$(LD) $(LDFLAGS) -o $(BIN) build/boot.o build/kernel.o
	grub-file --is-x86-multiboot $(BIN)

$(ISO): $(BIN)
	mkdir -p build/iso/boot/grub
	cp $(BIN) build/iso/boot/kernel.bin
	cp $(GRUB_CFG) build/iso/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) build/iso

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

clean:
	rm -rf build

.PHONY: all run clean
