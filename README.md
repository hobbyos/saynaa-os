README - Simple x86 Bootloader "Hello, World!"

This is a minimal 16-bit x86 bootloader written in NASM assembly.
When booted, it prints:

Hello, world from bootloader!

on the screen using BIOS interrupts.

-----------------------------------------
Project Structure

```txt
project/
├── Makefile
├── README.md
└── src/
    └── boot.asm
```

- src/boot.asm   : Bootloader source code (512 bytes)
- Makefile       : Automates building, running, and cleaning
- build/         : Folder where compiled binaries are placed

-----------------------------------------
Requirements

Tools needed:

- nasm               : Assembler
- qemu-system-x86_64 : Emulator
- make               : Build automation (usually preinstalled)

Install on Linux:
Debian/Ubuntu: sudo apt install nasm qemu-system make
Arch:          sudo pacman -S nasm qemu make
Gentoo:       sudo emerge nasm qemu make

-----------------------------------------
Build Instructions

Run:
```bash
make
```
This creates the bootloader binary at:

```build/boot.bin```

-----------------------------------------
Run in QEMU

Run:
```bash
make run
```

You should see:

Hello, world from bootloader!

Exit QEMU with Ctrl + C or by closing the window.

-----------------------------------------
Clean Build Files

Run:
```bash
make clean
```

This removes the build/ directory and generated files.

-----------------------------------------
How It Works

- BIOS loads first 512 bytes of the boot device at 0x7C00.
- boot.asm code:
  1. Initializes segment registers.
  2. Loads each character from the string using 'lodsb'.
  3. Prints via BIOS INT 10h (teletype).
  4. Stops CPU with 'hlt'.

-----------------------------------------
Bootloader Layout (512 bytes)

Bytes 0–509   : Boot code and data
Bytes 510–511 : Boot signature 0x55AA (required by BIOS)

-----------------------------------------
License

MIT License — free to use, modify, and distribute.

-----------------------------------------
