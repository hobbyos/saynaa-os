# Simple Makefile for x86 bootloader

SRC = src/boot.asm
BIN = build/boot.bin

ASM = nasm
QEMU = qemu-system-x86_64

all: $(BIN)

build:
	mkdir -p build

$(BIN): $(SRC) | build
	$(ASM) -f bin $(SRC) -o $(BIN)

run: $(BIN)
	$(QEMU) -drive format=raw,file=$(BIN) -boot a

clean:
	rm -rf build

.PHONY: all run clean
