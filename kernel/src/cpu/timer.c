
#include "kernel/cpu/timer.h"

#include "kernel/cpu/isr.h"
#include "kernel/cpu/ports.h"
#include "kernel/lib/kprintf.h"

// Globals are always initialized to 0
static uint32_t current_tick;
ISR callback;

void init_timer() {
    uint32_t divisor = TIMER_QUOTIENT / TIMER_FREQ;

    outportb(PIT_CMD, PIT_SET);
    outportb(PIT_0, divisor & 0xFF);
    outportb(PIT_0, (divisor >> 8) & 0xFF);

    isr_register_handler(32, &timer_callback);
}

void timer_callback(REGISTERS* regs) {
    current_tick++;

    if (callback) {
        callback(regs);
    }
}

uint32_t timer_get_tick() {
    return current_tick;
}

/* Returns the time since boot in seconds
 */
double timer_get_time() {
    return current_tick * (1.0 / TIMER_FREQ);
}

void timer_register_callback(ISR handler) {
    if (callback) {
        kprintf("[TIMER] Callback already registered");
        kprintf("[TIMER] Add support for multiple callbacks!");
    } else {
        callback = handler;
    }
}
