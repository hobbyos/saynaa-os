#include "kernel/sys/syscall.h"

#include "kernel/cpu/isr.h"
#include "kernel/cpu/timer.h"
#include "kernel/kernel.h"
#include "kernel/lib/kprintf.h"
#include "kernel/sys/proc.h"
#include "libc/stdio.h"
#include "libc/stdlib.h"

static void syscall_handler(REGISTERS* regs);

static void syscall_yield(REGISTERS* regs);
static void syscall_exit(REGISTERS* regs);
static void syscall_wait(REGISTERS* regs);
static void syscall_putchar(REGISTERS* regs);

sys_handler_t syscall_handlers[SYSCALL_NUM] = {0};

void init_syscall() {
    isr_register_handler(48, &syscall_handler);

    syscall_handlers[1] = syscall_exit;
    syscall_handlers[2] = syscall_putchar;
}

static void syscall_handler(REGISTERS* regs) {
    if (syscall_handlers[regs->eax]) {
        sys_handler_t handler = syscall_handlers[regs->eax];
        handler(regs);
    } else {
        kprintf("Unknown syscall %d\n", regs->eax);
    }
}

static void syscall_exit(REGISTERS* regs) {
    unused(regs);
    proc_exit();
}

static void syscall_putchar(REGISTERS* regs) {
    vbe_print_char((char) regs->ebx);
}
