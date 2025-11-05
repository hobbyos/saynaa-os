section .text
align 4

extern current_process
extern set_kernel_stack

global proc_switch_process
proc_switch_process:         ; void proc_switch_process(process_t* next)
    ; Save register state
    push ebx
    push esi
    push edi
    push ebp

    ; eax = current_process
    mov eax, [current_process]
    ; current_process->esp = esp
    mov [eax + 20], esp

    ; eax = next
    ; current_process = next
    mov eax, [esp + 20]
    mov [current_process], eax

    ; Set esp0 to the next process's kernel stack in the TSS
    push eax
    push dword [eax + 16]     ; kernel_stack
    call set_kernel_stack
    add esp, 4
    pop eax

    ; Switch to the next process's saved kernel stack
    mov esp, [eax + 20]

    ; Switch page directory
    mov ebx, [eax + 12]       ; directory
    mov cr3, ebx

    ; Restore registers from the next process's kernel stack
    pop ebp
    pop edi
    pop esi
    pop ebx

    ret
