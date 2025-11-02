# Saynaa hobby os

This is a minimal 32-bit x86 hobby os

-----------------------------------------
Requirements

Tools needed:

- binutils           : Binary utilities (like ld, objcopy, etc.)
- nasm               : Assembler
- qemu-system-i386   : Emulator
- make               : Build automation
- clang              : C compiler
- grub               : Bootloader used to create a bootable ISO for your OS.
- xorriso            : Required by GRUB to generate bootable ISO images.
- gdb               : Debugger (optional, for debugging purposes)


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
make qemu
```

-----------------------------------------
License

MIT License — free to use, modify, and distribute.

-----------------------------------------
