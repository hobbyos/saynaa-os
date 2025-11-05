#pragma once

#include "kernel/cpu/isr.h"
#include "libc/stdint.h"

#define SYSCALL_NUM 256

typedef void (*sys_handler_t)(REGISTERS*);

void init_syscall();
