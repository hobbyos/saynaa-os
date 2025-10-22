BITS 32

; -------------------------------
; Multiboot constants
; -------------------------------
MB_MAGIC    equ 0x1BADB002
MB_FLAGS    equ 0
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)

; -------------------------------
; Multiboot header section
; -------------------------------
section .multiboot
    align 4
    dd MB_MAGIC
    dd MB_FLAGS
    dd MB_CHECKSUM

; -------------------------------
; Kernel entry point
; -------------------------------
section .text
global _start
extern kernel_main

_start:
    call kernel_main     ; jump to C kernel
    cli                  ; disable interrupts
    hlt                  ; halt CPU
    jmp $                ; infinite loop
