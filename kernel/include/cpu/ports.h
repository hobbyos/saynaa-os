#pragma once

#include "libc/stdint.h"

/**
 * Reads a byte from the specified port.
 *
 * @param port The port number to read from.
 * @return The byte read from the port.
 */
uint8_t inportb(uint16_t port);

/**
 * Writes a byte to the specified port.
 *
 * @param port The port number to write to.
 * @param val The byte to write to the port.
 */
void outportb(uint16_t port, uint8_t val);

/**
 * Reads 2 bytes (a short) from the specified port.
 *
 * @param port The port number to read from.
 * @return The 2 bytes read from the port.
 */
uint16_t inports(uint16_t port);

/**
 * Writes 2 bytes (a short) to the specified port.
 *
 * @param port The port number to write to.
 * @param data The 2 bytes to write to the port.
 */
void outports(uint16_t port, uint16_t data);

/**
 * Reads 4 bytes (a long) from the specified port.
 *
 * @param port The port number to read from.
 * @return The 4 bytes read from the port.
 */
uint32_t inportl(uint16_t port);

/**
 * Writes 4 bytes (a long) to the specified port.
 *
 * @param port The port number to write to.
 * @param data The 4 bytes to write to the port.
 */
void outportl(uint16_t port, uint32_t data);