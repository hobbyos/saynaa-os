#include "libc/string.h"

void memset(void* ptr, int value, uint32_t size) {
    unsigned char* buff = (unsigned char*) ptr;
    for (uint32_t i = 0; i < size; i++) {
        buff[i] = value;
    }
}