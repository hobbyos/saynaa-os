section .text
    extern isr_irq_handler
    extern isr_exception_handler

exception_handler:
    pusha                 ; push all registers
    mov ax, ds
    push eax              ; save ds
    
    mov ax, 0x10          ; load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_exception_handler

    pop ebx             ; restore kernel data segment
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popa                ; restore all registers
    add esp, 0x8        ; restore stack for erro no been pushed

    sti                 ; re-enable interrupts
    iret

; Only interrupts 8, 10, 11, 12, 13, 14 and 17 push an error code
%macro exception_error 1
  global exception_%1
  exception_%1:
    cli
    push %1   ; push exception number index in IDT
    jmp exception_handler
%endmacro

%macro exception_noerr 1
  global exception_%1
  exception_%1:
    cli
    push byte 0    ; store default err code(0)
    push %1   ; push exception number index in IDT
    jmp exception_handler
%endmacro

exception_noerr 0
exception_noerr 1
exception_noerr 2
exception_noerr 3
exception_noerr 4
exception_noerr 5
exception_noerr 6
exception_noerr 7
exception_error 8
exception_noerr 9
exception_error 10
exception_error 11
exception_error 12
exception_error 13
exception_error 14
exception_noerr 15
exception_noerr 16
exception_error 17
exception_noerr 18
exception_noerr 19
exception_noerr 20
exception_noerr 21
exception_noerr 22
exception_noerr 23
exception_noerr 24
exception_noerr 25
exception_noerr 26
exception_noerr 27
exception_noerr 28
exception_noerr 29
exception_noerr 30
exception_noerr 31
exception_noerr 128

irq_handler:
    pusha                 ; push all registers
    mov ax, ds
    push eax              ; save ds

    mov ax, 0x10          ; load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call isr_irq_handler
    pop esp

    pop ebx                ; restore kernel data segment
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popa                ; restore all registers
    add esp, 0x8        ; restore stack for erro no been pushed

    sti                 ; re-enable interrupts
    iret


%macro IRQ 2
  global irq_%1
  irq_%1:
    cli
    push byte 0
    push byte %2
    jmp irq_handler
%endmacro


IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47
IRQ 16, 48

; jumped to on first context switch
global irq_handler_end
irq_handler_end:

    pop gs
    pop fs
    pop es
    pop ds
    popa

    add esp, 8    ; Adjust stack pointer (for `irqN`)

    iret          ; Return from interrupt
