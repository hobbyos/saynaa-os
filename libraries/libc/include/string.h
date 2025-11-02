#pragma once

#include "libc/stdint.h"

/**
 * @brief Sets the first `size` bytes of the memory area pointed to by `ptr`
 *        to the specified `value`.
 *
 * @param ptr Pointer to the memory area to be filled.
 * @param value Value to be set (converted to an unsigned char).
 * @param size Number of bytes to be set to the value.
 */
void memset(void* ptr, int value, uint32_t size);

/**
 * @brief Copies `n` bytes from memory area `src` to memory area `dst`.
 *
 * @param dst Pointer to the destination memory area.
 * @param src Pointer to the source memory area.
 * @param n Number of bytes to copy.
 */
void memcpy(void* dst, const void* src, uint32_t n);

/**
 * @brief Computes the length of the string `s`, excluding the null terminator.
 *
 * @param s Pointer to the null-terminated string.
 * @return Length of the string.
 */
int strlen(const char* s);

/**
 * @brief Compares up to `n` characters of the string `s1` and `s2`.
 *
 * @param s1 Pointer to the first string.
 * @param s2 Pointer to the second string.
 * @param n Maximum number of characters to compare.
 * @return An integer less than, equal to, or greater than zero if `s1` is
 *         found, respectively, to be less than, to match, or be greater than `s2`.
 */
int strncmp(const char* s1, const char* s2, size_t n);

/**
 * @brief Locates the last occurrence of character `c` in the string `s`.
 *
 * @param s Pointer to the null-terminated string.
 * @param c Character to be located.
 * @return Pointer to the last occurrence of `c` in `s`, or NULL if not found.
 */
char* strrchr(const char* s, int c);