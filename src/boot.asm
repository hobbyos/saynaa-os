BITS 32

; -------------------------------
; Multiboot2 constants
; -------------------------------
%define MB2_MAGIC    0xE85250D6
%define MB2_ARCH     0
%define MB2_FLAGS    0      ; minimal, no special features
; Checksum: -(magic + arch + flags + header length)
; header length = end - start, calculated automatically later

; Multiboot2 tags
%define TAG_END       0

; -------------------------------
; Multiboot2 header
; -------------------------------
section .multiboot
align 8
multiboot_start:
    dd MB2_MAGIC
    dd MB2_ARCH
    dd MB2_FLAGS
    dd -(MB2_MAGIC + MB2_ARCH + MB2_FLAGS)  ; checksum

    ; End tag (required)
align 8
    dw TAG_END
    dw 0      ; flags
    dd 8      ; size of end tag
multiboot_end:

; -------------------------------
; Bootstrap stack
; -------------------------------
section .bss
align 16
stack_bottom:
    resb 16384     ; 16 KB stack
stack_top:

; -------------------------------
; Kernel entry
; -------------------------------
section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top   ; initialize stack

    ; Transfer control to C kernel
    push dword 0         ; multiboot info pointer (optional)
    push dword MB2_MAGIC ; multiboot magic number
    call kernel_main

    ; If kernel_main returns, halt CPU
    cli
.hang:
    hlt
    jmp .hang
