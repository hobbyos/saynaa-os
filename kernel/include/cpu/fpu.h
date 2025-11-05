#pragma once

#include "kernel/sys/proc.h"
#include "libc/stdint.h"

void init_fpu();
void fpu_switch(process_t* prev, const process_t* next);
void fpu_kernel_enter();
void fpu_kernel_exit();