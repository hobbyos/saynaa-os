section .data
prt: db "usermode> hello world!", 10, 10, 0    ; add null terminator for loop end check

section .text
align 4
global _start

_start:
    mov esi, prt          ; ESI points to start of string

print_loop:
    mov bl, [esi]         ; load one byte (character)
    cmp bl, 0             ; check if null terminator (end of string)
    je done               ; if yes, jump to done

    mov eax, 2            ; syscall number (print char)
    int 0x30              ; interrupt (your OS-specific syscall)

    inc esi               ; move to next character
    jmp print_loop        ; repeat

done:
hang:
    jmp hang              ; infinite loop, because this is last process

    mov eax, 1            ; syscall number (exit)
    int 0x30              ; exit program
