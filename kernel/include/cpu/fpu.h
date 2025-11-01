#pragma once

#include "libc/stdint.h"


void init_fpu();
void fpu_kernel_enter();
void fpu_kernel_exit();