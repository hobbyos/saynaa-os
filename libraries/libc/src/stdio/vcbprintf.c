
#include "libc/stdio.h"
#include "libc/stdlib.h"
#include "libc/string.h"

/* Max safety bounds */
#define MAX_FORMAT_SCAN 4096U
#define MAX_STRING_SCAN 8192U

static size_t convert_integer(char* destination, uintmax_t value, uintmax_t base, const char* digits) {
    size_t result = 1;
    uintmax_t copy = value;
    while (base <= copy)
        copy /= base, result++;
    for (size_t i = result; i != 0; i--) {
        destination[i - 1] = digits[value % base];
        value /= base;
    }
    return result;
}

static size_t convert_float(char* destination, double value, uintmax_t base, const char* digits, size_t precision) {
    size_t length = 0;

    // Handle negative numbers
    if (value < 0) {
        destination[length++] = '-';
        value = -value;
    }

    // Convert integer part
    uintmax_t int_part = (uintmax_t) value;
    double frac_part = value - int_part;

    length += convert_integer(destination + length, int_part, base, digits);

    // Handle fractional part
    if (precision > 0) {
        destination[length++] = '.'; // decimal point
        for (size_t i = 0; i < precision; i++) {
            frac_part *= base;
            uintmax_t digit = (uintmax_t) frac_part;
            destination[length++] = digits[digit % base];
            frac_part -= digit;
        }
    }

    return length;
}

static size_t noop_callback(void* ctx, const char* str, size_t amount) {
    (void) ctx;
    (void) str;
    return amount;
}

size_t strnlen_limited(const char* s, size_t max) {
    size_t i;
    for (i = 0; i < max && s[i]; i++)
        ;
    return i;
};

int vcbprintf(void* ctx, size_t (*callback)(void*, const char*, size_t), const char* format, va_list parameters) {
    if (!callback)
        return -1; /* no callback -> nowhere to write */

    size_t written = 0;
    bool rejected_bad_specifier = false;

    while (format && *format != '\0') {
        /* Non-format run: print until next '%' or end. Bounded scan to avoid walking off into invalid memory. */
        if (*format != '%') {
            const char* start = format;
            size_t amount = 0;
            /* bounded scan */
            while (amount < MAX_FORMAT_SCAN && start[amount] && start[amount] != '%')
                amount++;
            if (amount == 0) { /* should not happen, but be defensive */
                format++;
                continue;
            }
            if (amount >= MAX_FORMAT_SCAN)
                return -1; /* malformed / too long */
            size_t cbret = callback(ctx, start, amount);
            if (cbret != amount)
                return -1;
            format += amount;
            written += amount;
            continue;
        }

        /* At '%': handle special cases */
        const char* format_begun_at = format;

        /* Handle literal "%%" -> print single '%' */
        if (format[1] == '%') {
            if (callback(ctx, "%", 1) != 1)
                return -1;
            format += 2;
            written++;
            continue;
        }

        /* Advance past the '%' to parse flags/width/etc. */
        format++; /* now points at first char after '%' */

        if (rejected_bad_specifier) {
            /* If a previous bad specifier was rejected, back up and print the literal */
            rejected_bad_specifier = true;
            format = format_begun_at;
            /* print as non-format run (a single char or more) */
            if (callback(ctx, format, 1) != 1)
                return -1;
            format++;
            written++;
            continue;
        }

        /* FLAGS */
        bool alternate = false;
        bool zero_pad = false;
        bool field_width_is_negative = false;
        bool prepend_blank_if_positive = false;
        bool prepend_plus_if_positive = false;
        bool group_thousands = false;
        bool alternate_output_digits = false;

        while (true) {
            char f = *format;
            if (!f)
                break;
            if (f == '#') {
                alternate = true;
                format++;
                continue;
            }
            if (f == '0') {
                zero_pad = true;
                format++;
                continue;
            }
            if (f == '-') {
                field_width_is_negative = true;
                format++;
                continue;
            }
            if (f == ' ') {
                prepend_blank_if_positive = true;
                format++;
                continue;
            }
            if (f == '+') {
                prepend_plus_if_positive = true;
                format++;
                continue;
            }
            if (f == '\'') {
                group_thousands = true;
                format++;
                continue;
            }
            if (f == 'I') {
                alternate_output_digits = true;
                format++;
                continue;
            }
            break;
        }

        (void) group_thousands;
        (void) alternate_output_digits;

        /* FIELD WIDTH */
        int field_width = 0;
        if (*format == '*') {
            format++;
            field_width = va_arg(parameters, int);
        } else {
            /* parse decimal number, bounded */
            size_t count = 0;
            while (count < MAX_FORMAT_SCAN && '0' <= *format && *format <= '9') {
                field_width = 10 * field_width + (*format - '0');
                format++;
                count++;
            }
            if (count >= MAX_FORMAT_SCAN)
                return -1;
        }
        if (field_width_is_negative)
            field_width = -field_width;
        size_t abs_field_width = (size_t) (field_width < 0 ? -field_width : field_width);

        /* PRECISION */
        size_t precision = SIZE_MAX;
        if (*format == '.') {
            format++;
            if (*format == '*') {
                format++;
                int int_precision = va_arg(parameters, int);
                precision = (int_precision >= 0) ? (size_t) int_precision : 0;
            } else if (*format == '-') {
                /* skip digits after '-' (C's behavior: negative precision ignored) */
                format++;
                while ('0' <= *format && *format <= '9') {
                    format++;
                }
                precision = 0;
            } else {
                size_t prec = 0;
                size_t count = 0;
                while (count < MAX_FORMAT_SCAN && '0' <= *format && *format <= '9') {
                    prec = 10 * prec + (*format - '0');
                    format++;
                    count++;
                }
                if (count >= MAX_FORMAT_SCAN)
                    return -1;
                precision = prec;
            }
        }

        /* LENGTH MODIFIERS */
        enum length_t {
            LENGTH_SHORT_SHORT,
            LENGTH_SHORT,
            LENGTH_DEFAULT,
            LENGTH_LONG,
            LENGTH_LONG_LONG,
            LENGTH_LONG_DOUBLE,
            LENGTH_INTMAX_T,
            LENGTH_SIZE_T,
            LENGTH_PTRDIFF_T,
        };

        struct length_modifier {
            const char* name;
            enum length_t length;
        };
        struct length_modifier length_modifiers[] = {
            {"hh", LENGTH_SHORT_SHORT},
            {"h", LENGTH_SHORT},
            {"", LENGTH_DEFAULT},
            {"l", LENGTH_LONG},
            {"ll", LENGTH_LONG_LONG},
            {"L", LENGTH_LONG_DOUBLE},
            {"j", LENGTH_INTMAX_T},
            {"z", LENGTH_SIZE_T},
            {"t", LENGTH_PTRDIFF_T},
        };

        enum length_t length = LENGTH_DEFAULT;
        size_t best_len = 0;
        for (size_t i = 0; i < sizeof(length_modifiers) / sizeof(length_modifiers[0]); i++) {
            size_t nl = strlen(length_modifiers[i].name);
            if (nl < best_len)
                continue;
            if (strncmp(format, length_modifiers[i].name, nl) != 0)
                continue;
            length = length_modifiers[i].length;
            best_len = nl;
        }
        format += best_len;

        /* CONVERSION CHAR (we check ranges safely) */
        char conv = *format;
        if (!conv) { /* string ended after specifier start -> treat as literal */
            format = format_begun_at;
            if (callback(ctx, format, 1) != 1)
                return -1;
            format++;
            written++;
            continue;
        }

        /* Integer / pointer conversions */
        if (conv == 'd' || conv == 'i' || conv == 'o' || conv == 'u' || conv == 'x' || conv == 'X' || conv == 'p') {
            format++; /* consume conversion */
            char conversion = conv;
            bool negative_value = false;
            uintmax_t value = 0;

            if (conversion == 'p') {
                void* ptr = va_arg(parameters, void*);
                value = (uintptr_t) ptr;
                conversion = 'x';
                alternate = !alternate;
                prepend_blank_if_positive = false;
                prepend_plus_if_positive = false;
            } else if (conversion == 'i' || conversion == 'd') {
                intmax_t signed_value = 0;
                switch (length) {
                case LENGTH_SHORT_SHORT:
                case LENGTH_SHORT:
                case LENGTH_DEFAULT:
                    signed_value = va_arg(parameters, int);
                    break;
                case LENGTH_LONG:
                    signed_value = va_arg(parameters, long);
                    break;
                case LENGTH_LONG_LONG:
                    signed_value = va_arg(parameters, long long);
                    break;
                case LENGTH_INTMAX_T:
                    signed_value = va_arg(parameters, intmax_t);
                    break;
                case LENGTH_SIZE_T:
                    /* size_t is unsigned — using cast as in your original: treat as signed */
                    signed_value = (intmax_t) va_arg(parameters, size_t);
                    break;
                case LENGTH_PTRDIFF_T:
                    signed_value = (intmax_t) va_arg(parameters, ptrdiff_t);
                    break;
                default:
                    goto bad_conv;
                }
                if (signed_value < 0) {
                    negative_value = true;
                    /* careful with INT_MIN: cast to uintmax_t first */
                    value = (uintmax_t) (-(intmax_t) signed_value);
                } else {
                    value = (uintmax_t) signed_value;
                }
            } else {
                /* unsigned conversions */
                switch (length) {
                case LENGTH_SHORT_SHORT:
                case LENGTH_SHORT:
                case LENGTH_DEFAULT:
                    value = (uintmax_t) va_arg(parameters, unsigned int);
                    break;
                case LENGTH_LONG:
                    value = (uintmax_t) va_arg(parameters, unsigned long);
                    break;
                case LENGTH_LONG_LONG:
                    value = (uintmax_t) va_arg(parameters, unsigned long long);
                    break;
                case LENGTH_INTMAX_T:
                    value = va_arg(parameters, uintmax_t);
                    break;
                case LENGTH_SIZE_T:
                    value = (uintmax_t) va_arg(parameters, size_t);
                    break;
                case LENGTH_PTRDIFF_T:
                    value = (uintmax_t) (va_arg(parameters, ptrdiff_t));
                    break;
                default:
                    goto bad_conv;
                }
                prepend_blank_if_positive = false;
                prepend_plus_if_positive = false;
            }

            /* prepare digits and base */
            const char* digits = (conversion == 'X') ? "0123456789ABCDEF" : "0123456789abcdef";
            uintmax_t base = (conversion == 'x' || conversion == 'X') ? 16
                             : (conversion == 'o')                    ? 8
                                                                      : 10;

            char prefix[8];
            size_t prefix_digits_length = 0;
            size_t prefix_length = 0;

            if (negative_value) {
                prefix[prefix_length++] = '-';
                prefix_digits_length++;
            } else if (prepend_plus_if_positive) {
                prefix[prefix_length++] = '+';
                prefix_digits_length++;
            } else if (prepend_blank_if_positive) {
                prefix[prefix_length++] = ' ';
                prefix_digits_length++;
            }

            if (alternate && (conversion == 'x' || conversion == 'X') && value != 0) {
                prefix[prefix_digits_length++] = '0';
                prefix[prefix_digits_length++] = conversion;
                prefix_length = prefix_digits_length;
            } else if (alternate && conversion == 'o' && value != 0) {
                prefix[prefix_digits_length++] = '0';
                prefix_length = prefix_digits_length;
            } else {
                prefix_length = prefix_digits_length;
            }

            char output[sizeof(uintmax_t) * 3 + 4];
            size_t output_length = convert_integer(output, value, base, digits);

            if (precision == 0 && output_length == 1 && output[0] == '0') {
                output_length = 0;
                output[0] = '\0';
            }
            size_t output_length_with_precision = (precision != SIZE_MAX && output_length < precision)
                                                      ? precision
                                                      : output_length;

            size_t digits_length = prefix_digits_length + output_length;
            size_t normal_length = prefix_length + output_length;
            size_t length_with_precision = prefix_length + output_length_with_precision;

            bool use_precision = (precision != SIZE_MAX);
            bool use_zero_pad = zero_pad && field_width >= 0 && !use_precision;
            bool use_left_pad = !use_zero_pad && field_width >= 0;
            bool use_right_pad = !use_zero_pad && field_width < 0;

            if (use_left_pad) {
                for (size_t i = length_with_precision; i < abs_field_width; i++) {
                    if (callback(ctx, " ", 1) != 1)
                        return -1;
                    written++;
                }
            }

            if (prefix_length) {
                if (callback(ctx, prefix, prefix_length) != prefix_length)
                    return -1;
                written += prefix_length;
            }

            if (use_zero_pad) {
                for (size_t i = normal_length; i < abs_field_width; i++) {
                    if (callback(ctx, "0", 1) != 1)
                        return -1;
                    written++;
                }
            }

            if (use_precision) {
                for (size_t i = digits_length; i < precision; i++) {
                    if (callback(ctx, "0", 1) != 1)
                        return -1;
                    written++;
                }
            }

            if (output_length) {
                if (callback(ctx, output, output_length) != output_length)
                    return -1;
                written += output_length;
            }

            if (use_right_pad) {
                for (size_t i = length_with_precision; i < abs_field_width; i++) {
                    if (callback(ctx, " ", 1) != 1)
                        return -1;
                    written++;
                }
            }
            continue;
        }

        /* Floating point conversions */
        if (conv == 'e' || conv == 'E' || conv == 'f' || conv == 'F' || conv == 'g' || conv == 'G'
            || conv == 'a' || conv == 'A') {
            format++; /* consume conversion */
            long double ldval;
            if (length == LENGTH_DEFAULT)
                ldval = va_arg(parameters, double);
            else if (length == LENGTH_LONG_DOUBLE)
                ldval = va_arg(parameters, long double);
            else
                goto bad_conv;

            /* buffer and conversion (reasonable sized buffer) */
            char float_buffer[256];
            size_t precision_value = (precision == SIZE_MAX) ? 6 : precision;
            size_t float_length = convert_float(float_buffer, (double) ldval, 10, "0123456789", precision_value);

            size_t total_length = float_length;
            bool negative_value = ((double) ldval < 0.0);

            if (negative_value || prepend_plus_if_positive || prepend_blank_if_positive) {
                char sign = negative_value ? '-' : (prepend_plus_if_positive ? '+' : ' ');
                if (callback(ctx, &sign, 1) != 1)
                    return -1;
                total_length++;
            }

            if (abs_field_width > total_length && !field_width_is_negative) {
                for (size_t i = total_length; i < abs_field_width; i++) {
                    if (callback(ctx, " ", 1) != 1)
                        return -1;
                    written++;
                }
            }

            if (callback(ctx, float_buffer, float_length) != float_length)
                return -1;
            written += float_length;

            if (field_width_is_negative && abs_field_width > total_length) {
                for (size_t i = total_length; i < abs_field_width; i++) {
                    if (callback(ctx, " ", 1) != 1)
                        return -1;
                    written++;
                }
            }
            continue;
        }

        /* Character conversion */
        if (conv == 'c') {
            format++; /* consume */
            char c;
            if (length == LENGTH_DEFAULT)
                c = (char) va_arg(parameters, int);
            else if (length == LENGTH_LONG) {
                /* wide char not supported */
                (void) va_arg(parameters, uint32_t);
                /* unsupported - print nothing or treat as error */
                goto bad_conv;
            } else {
                goto bad_conv;
            }

            if (!field_width_is_negative && 1 < abs_field_width) {
                for (size_t i = 1; i < abs_field_width; i++) {
                    if (callback(ctx, " ", 1) != 1)
                        return -1;
                    written++;
                }
            }
            if (callback(ctx, &c, 1) != 1)
                return -1;
            written++;

            if (field_width_is_negative && 1 < abs_field_width) {
                for (size_t i = 1; i < abs_field_width; i++) {
                    if (callback(ctx, " ", 1) != 1)
                        return -1;
                    written++;
                }
            }
            continue;
        }

        /* String conversions */
        if (conv == 's' || conv == 'm') {
            format++; /* consume */
            const char* string;
            if (length == LENGTH_DEFAULT)
                string = va_arg(parameters, const char*);
            else if (length == LENGTH_LONG) {
                (void) va_arg(parameters, const int*);
                goto bad_conv;
            } else {
                goto bad_conv;
            }

            if (conv == 's' && !string)
                string = "(null)";

            /* bounded string length calculation */
            size_t max_scan = (precision == SIZE_MAX)
                                  ? MAX_STRING_SCAN
                                  : (precision < MAX_STRING_SCAN ? precision : MAX_STRING_SCAN);
            size_t string_length = strnlen_limited(string, max_scan);

            if (!field_width_is_negative && string_length < abs_field_width) {
                for (size_t i = string_length; i < abs_field_width; i++) {
                    if (callback(ctx, " ", 1) != 1)
                        return -1;
                    written++;
                }
            }

            if (callback(ctx, string, string_length) != string_length)
                return -1;
            written += string_length;

            if (field_width_is_negative && string_length < abs_field_width) {
                for (size_t i = string_length; i < abs_field_width; i++) {
                    if (callback(ctx, " ", 1) != 1)
                        return -1;
                    written++;
                }
            }
            continue;
        }

        /* 'n' specifier: store number of characters written so far */
        if (conv == 'n') {
            format++; /* consume */
            if (length == LENGTH_SHORT_SHORT) {
                signed char* p = va_arg(parameters, signed char*);
                if (!p)
                    return -1;
                *p = (signed char) written;
            } else if (length == LENGTH_SHORT) {
                short* p = va_arg(parameters, short*);
                if (!p)
                    return -1;
                *p = (short) written;
            } else if (length == LENGTH_DEFAULT) {
                int* p = va_arg(parameters, int*);
                if (!p)
                    return -1;
                *p = (int) written;
            } else if (length == LENGTH_LONG) {
                long* p = va_arg(parameters, long*);
                if (!p)
                    return -1;
                *p = (long) written;
            } else if (length == LENGTH_LONG_LONG) {
                long long* p = va_arg(parameters, long long*);
                if (!p)
                    return -1;
                *p = (long long) written;
            } else if (length == LENGTH_INTMAX_T) {
                uintmax_t* p = va_arg(parameters, uintmax_t*);
                if (!p)
                    return -1;
                *p = (uintmax_t) written;
            } else if (length == LENGTH_SIZE_T) {
                size_t* p = va_arg(parameters, size_t*);
                if (!p)
                    return -1;
                *p = (size_t) written;
            } else if (length == LENGTH_PTRDIFF_T) {
                ptrdiff_t* p = va_arg(parameters, ptrdiff_t*);
                if (!p)
                    return -1;
                *p = (ptrdiff_t) written;
            } else {
                goto bad_conv;
            }
            continue;
        }

        /* If we reach here, conversion is not understood */
    bad_conv:
        /* Rewind to initial '%' and print it literally to avoid infinite loop */
        format = format_begun_at;
        if (callback(ctx, "%", 1) != 1)
            return -1;
        format++;
        written++;
        rejected_bad_specifier = true;
    }

    if (written > (size_t) INT32_MAX)
        return -1;
    return (int) written;
}