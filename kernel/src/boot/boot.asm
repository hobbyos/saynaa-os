BITS 32

; -------------------------------
; Multiboot2 constants
; -------------------------------
%define MB2_MAGIC    0xE85250D6
%define MB2_ARCH     0      ; 0 for x86
%define MB2_HEADER   multiboot_end - multiboot_start
%define MB2_CHECK    -(MB2_MAGIC + MB2_ARCH + MB2_HEADER)

; Multiboot2 tags
%define TAG_END          0
%define TAG_FRAMEBUFFER  5
%define TAG_REQUIRED     0


; -------------------------------
; Multiboot2 header
; -------------------------------
section .multiboot
multiboot_start:
align 8
    dd MB2_MAGIC
    dd MB2_ARCH
    dd MB2_HEADER
    dd MB2_CHECK

    ; Graphics framebuffer tag (for setting resolution)
align 8
    dw TAG_FRAMEBUFFER  ; Tag type
    dw TAG_REQUIRED     ; Required tag
    dd 20               ; Total size of the tag
    dd 1024             ; Width (1024px)
    dd 768              ; Height (768px)
    dd 32               ; Bits per pixel (32bpp)

    ; End tag (required)
align 8
    dw TAG_END
    dw TAG_REQUIRED
    dd 8
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
    mov ebp, 0            ; Stop stack traces here

    ; Transfer control to C kernel
    push eax
    push ebx
    cli               ; Disable interrupts
    call kernel_main
    add esp, 8

    ; If kernel_main returns, halt CPU
    cli
.hang:
    hlt
    jmp .hang
