#include "libc/stdlib.h"

int abs(int num) {
    return num < 0 ? -num : num;
}
