#pragma once

#include "libc/stdint.h"

// Initialize the serial port (configure baud rate and settings)
void init_serial();

// Read a single byte from the serial port (blocking)
char read_serial();

// Write a single byte to the serial port
void write_serial(char a);