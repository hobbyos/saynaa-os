section .text
global load_gdt
global load_idt
global load_tss

load_gdt:
    mov eax, [esp + 4]
    lgdt [eax]

    ; Set the segment registers to the kernel data segment (selector 0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Enable protected mode
    cli
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Far jump to flush the instruction pipeline and update CS register
    jmp 0x08:far_jump

far_jump:
    ret

load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret

load_tss:
    mov ax, 0x2B
    ltr ax
    ret
