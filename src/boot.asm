; boot.asm - simple 16-bit boot sector that prints a message via BIOS teletype (INT 10h)
; Assemble with: nasm -f bin boot.asm -o boot.bin

org 0x7C00        ; BIOS loads the boot sector here
bits 16

start:
    cli                 ; disable interrupts (optional)
    xor ax, ax
    mov ds, ax
    mov es, ax

    mov si, msg         ; pointer to the string

.print_char:
    lodsb               ; load byte at [ds:si] -> AL, increments SI
    cmp al, 0
    je .hang
    mov ah, 0x0E        ; BIOS teletype function
    mov bh, 0x00        ; page
    mov bl, 0x07        ; attribute (text color) - ignored by int 0x10/ah=0Eh on many BIOSes
    int 0x10
    jmp .print_char

.hang:
    hlt                 ; halt CPU (loop is fine too)
    jmp .hang

; message (zero-terminated)
msg db 'Hello, world from bootloader!', 0

; pad to 510 bytes (boot signature goes in last two bytes)
times 510 - ($ - $$) db 0
dw 0xAA55            ; boot signature (little endian)
