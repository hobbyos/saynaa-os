# GRUB Multiboot Kernel — "Hello, world from GRUB kernel!"

This is a minimal 32-bit **GRUB Multiboot 1 kernel** written in C and assembly.  
When booted, it prints: ```Hello, world from GRUB kernel!``` to the screen.

-----------------------------------------
Project Structure

```txt
saynaa-os
├── LICENSE
├── linker.ld
├── Makefile
├── README.md
└── src
    ├── boot.asm
    ├── grub.cfg
    ├── kernel.c
    └── kernel.h
```

- Makefile       : Automates building, running, and cleaning
- build/         : Directory for build output
- linker.ld      : Linker script for kernel binary
- src/boot.asm   : grub bootloader assembly code
- src/grub.cfg   : GRUB configuration file
- src/kernel.c   : C source code for the kernel
- src/kernel.h   : Header file with type definitions and constants

-----------------------------------------
Requirements

Tools needed:

- nasm               : Assembler
- qemu-system-i386   : Emulator
- make               : Build automation (usually preinstalled)
- clang              : C compiler
- grub-mkrescue      : Create bootable ISO with GRUB

-----------------------------------------
Build Instructions

Run:
```bash
make
```
-----------------------------------------
Run in QEMU

Run:
```bash
make run
```

-----------------------------------------
Clean Build Files

Run:
```bash
make clean
```

-----------------------------------------
License

MIT License — free to use, modify, and distribute.

-----------------------------------------
