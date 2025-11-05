#include "libc/stdio.h"
#include "libc/string.h"

struct vsnprintf {
    char* str;
    size_t size;
    size_t written;
};

static size_t vsnprintf_callback(void* ctx, const char* string, size_t length) {
    struct vsnprintf* state = (struct vsnprintf*) ctx;
    if (state->size > 0 && state->written < state->size) {
        size_t available = state->size - state->written;
        size_t possible = length < available ? length : available;
        memcpy(state->str + state->written, string, possible);
        state->written += possible;
        return possible;
    }
    return 0; // wrote nothing
}

int vsnprintf(char* restrict str, size_t size, const char* restrict format, va_list list) {
    struct vsnprintf state;
    state.str = str;
    state.size = size ? size - 1 : 0;
    state.written = 0;
    int result = vcbprintf(&state, vsnprintf_callback, format, list);
    if (1 <= size)
        state.str[state.written] = '\0';
    return result;
}
