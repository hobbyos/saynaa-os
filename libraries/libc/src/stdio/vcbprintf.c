
#include "libc/stdio.h"
#include "libc/stdlib.h"
#include "libc/string.h"

static size_t convert_integer(char* destination, uintmax_t value, uintmax_t base, const char* digits);
static size_t convert_float(char* destination, double value, uintmax_t base, const char* digits, size_t precision);

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

int vcbprintf(void* ctx, size_t (*callback)(void*, const char*, size_t), const char* format, va_list parameters) {
    if (!callback)
        callback = noop_callback;

    size_t written = 0;
    bool rejected_bad_specifier = false;

    while (*format != '\0') {
        if (*format != '%') {
            size_t amount;
        print_c:
            amount = 1;
            while (format[amount] && format[amount] != '%')
                amount++;
            if (callback(ctx, format, amount) != amount)
                return -1;
            format += amount;
            written += amount;
            continue;
        }

        const char* format_begun_at = format;

        if (*(++format) == '%')
            goto print_c;

        if (rejected_bad_specifier) {
        incomprehensible_conversion:
            rejected_bad_specifier = true;
        unsupported_conversion:
            format = format_begun_at;
            goto print_c;
        }

        bool alternate = false;
        bool zero_pad = false;
        bool field_width_is_negative = false;
        bool prepend_blank_if_positive = false;
        bool prepend_plus_if_positive = false;
        bool group_thousands = false;
        bool alternate_output_digits = false;

        while (true) {
            switch (*format++) {
            case '#':
                alternate = true;
                continue;
            case '0':
                zero_pad = true;
                continue;
            case '-':
                field_width_is_negative = true;
                continue;
            case ' ':
                prepend_blank_if_positive = true;
                continue;
            case '+':
                prepend_plus_if_positive = true;
                continue;
            case '\'':
                group_thousands = true;
                continue;
            case 'I':
                alternate_output_digits = true;
                continue;
            default:
                format--;
                break;
            }
            break;
        }

        (void) group_thousands;
        (void) alternate_output_digits;

        int field_width = 0;
        if (*format == '*' && (format++, true))
            field_width = va_arg(parameters, int);
        else
            while ('0' <= *format && *format <= '9')
                field_width = 10 * field_width + *format++ - '0';
        if (field_width_is_negative)
            field_width = -field_width;
        size_t abs_field_width = (size_t) abs(field_width);

        size_t precision = SIZE_MAX;
        if (*format == '.' && (format++, true)) {
            precision = 0;
            if (*format == '*' && (format++, true)) {
                int int_precision = va_arg(parameters, int);
                precision = 0 <= int_precision ? (size_t) int_precision : 0;
            } else if (*format == '-' && (format++, true)) {
                while ('0' <= *format && *format <= '9')
                    format++;
            } else {
                while ('0' <= *format && *format <= '9')
                    precision = 10 * precision + *format++ - '0';
            }
        }

        enum length {
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

        struct length_modifer {
            const char* name;
            enum length length;
        };

        struct length_modifer length_modifiers[] = {
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

        enum length length = LENGTH_DEFAULT;
        size_t length_length = 0;
        for (size_t i = 0; i < sizeof(length_modifiers) / sizeof(length_modifiers[0]); i++) {
            size_t name_length = strlen(length_modifiers[i].name);
            if (name_length < length_length)
                continue;
            if (strncmp(format, length_modifiers[i].name, name_length) != 0)
                continue;
            length = length_modifiers[i].length;
            length_length = name_length;
        }

        format += length_length;

        if (*format == 'd' || *format == 'i' || *format == 'o' || *format == 'u' || *format == 'x'
            || *format == 'X' || *format == 'p') {
            char conversion = *format++;

            bool negative_value = false;
            uintmax_t value;
            if (conversion == 'p') {
                value = (uintptr_t) va_arg(parameters, void*);
                conversion = 'x';
                alternate = !alternate;
                prepend_blank_if_positive = false;
                prepend_plus_if_positive = false;
            } else if (conversion == 'i' || conversion == 'd') {
                intmax_t signed_value;
                if (length == LENGTH_SHORT_SHORT)
                    signed_value = va_arg(parameters, int);
                else if (length == LENGTH_SHORT)
                    signed_value = va_arg(parameters, int);
                else if (length == LENGTH_DEFAULT)
                    signed_value = va_arg(parameters, int);
                else if (length == LENGTH_LONG)
                    signed_value = va_arg(parameters, long);
                else if (length == LENGTH_LONG_LONG)
                    signed_value = va_arg(parameters, long long);
                else if (length == LENGTH_INTMAX_T)
                    signed_value = va_arg(parameters, intmax_t);
                else if (length == LENGTH_SIZE_T)
                    signed_value = va_arg(parameters, size_t);
                else if (length == LENGTH_PTRDIFF_T)
                    signed_value = va_arg(parameters, ptrdiff_t);
                else
                    goto incomprehensible_conversion;
                value = (negative_value = signed_value < 0) ? -(uintmax_t) signed_value : (uintmax_t) signed_value;
            } else {
                if (length == LENGTH_SHORT_SHORT)
                    value = va_arg(parameters, unsigned int);
                else if (length == LENGTH_SHORT)
                    value = va_arg(parameters, unsigned int);
                else if (length == LENGTH_DEFAULT)
                    value = va_arg(parameters, unsigned int);
                else if (length == LENGTH_LONG)
                    value = va_arg(parameters, unsigned long);
                else if (length == LENGTH_LONG_LONG)
                    value = va_arg(parameters, unsigned long long);
                else if (length == LENGTH_INTMAX_T)
                    value = va_arg(parameters, uintmax_t);
                else if (length == LENGTH_SIZE_T)
                    value = va_arg(parameters, size_t);
                else if (length == LENGTH_PTRDIFF_T)
                    value = (uintmax_t) va_arg(parameters, ptrdiff_t);
                else
                    goto incomprehensible_conversion;
                prepend_blank_if_positive = false;
                prepend_plus_if_positive = false;
            }

            const char* digits = conversion == 'X' ? "0123456789ABCDEF" : "0123456789abcdef";
            uintmax_t base = (conversion == 'x' || conversion == 'X') ? 16
                             : conversion == 'o'                      ? 8
                                                                      : 10;
            char prefix[3];
            size_t prefix_length = 0;
            size_t prefix_digits_length = 0;
            if (negative_value)
                prefix[prefix_length++] = '-';
            else if (prepend_plus_if_positive)
                prefix[prefix_length++] = '+';
            else if (prepend_blank_if_positive)
                prefix[prefix_length++] = ' ';
            if (alternate && (conversion == 'x' || conversion == 'X') && value != 0) {
                prefix[prefix_digits_length++, prefix_length++] = '0';
                prefix[prefix_digits_length++, prefix_length++] = conversion;
            }
            if (alternate && conversion == 'o' && value != 0)
                prefix[prefix_digits_length++, prefix_length++] = '0';

            char output[sizeof(uintmax_t) * 3];
            size_t output_length = convert_integer(output, value, base, digits);
            if (!precision && output_length == 1 && output[0] == '0') {
                output_length = 0;
                output[0] = '\0';
            }
            size_t output_length_with_precision = precision != SIZE_MAX && output_length < precision
                                                      ? precision
                                                      : output_length;

            size_t digits_length = prefix_digits_length + output_length;
            size_t normal_length = prefix_length + output_length;
            size_t length_with_precision = prefix_length + output_length_with_precision;

            bool use_precision = precision != SIZE_MAX;
            bool use_zero_pad = zero_pad && 0 <= field_width && !use_precision;
            bool use_left_pad = !use_zero_pad && 0 <= field_width;
            bool use_right_pad = !use_zero_pad && field_width < 0;

            if (use_left_pad) {
                for (size_t i = length_with_precision; i < abs_field_width; i++) {
                    if (callback(ctx, " ", 1) != 1)
                        return -1;
                    written++;
                }
            }
            if (callback(ctx, prefix, prefix_length) != prefix_length)
                return -1;
            written += prefix_length;
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
            if (callback(ctx, output, output_length) != output_length)
                return -1;
            written += output_length;
            if (use_right_pad) {
                for (size_t i = length_with_precision; i < abs_field_width; i++) {
                    if (callback(ctx, " ", 1) != 1)
                        return -1;
                    written++;
                }
            }
        } else if (*format == 'e' || *format == 'E' || *format == 'f' || *format == 'F'
                   || *format == 'g' || *format == 'G' || *format == 'a' || *format == 'A') {
            char conversion = *format++;

            long double value;
            if (length == LENGTH_DEFAULT)
                value = va_arg(parameters, double);
            else if (length == LENGTH_LONG_DOUBLE)
                value = va_arg(parameters, long double);
            else
                goto incomprehensible_conversion;

            // Prepare buffer
            // Buffer for float conversion
            // WARNING: This buffer is only safe for reasonably small floats and moderate precision (e.g., <100 decimal digits).
            // Large numbers or very high precision may overflow this buffer.
            char float_buffer[128]; // Enough for most practical uses
            size_t precision_value = precision == SIZE_MAX ? 6 : precision; // default precision: 6
            size_t float_length = convert_float(float_buffer, (double) value, 10, "0123456789", precision_value);

            // Handle sign and left/right padding
            size_t total_length = float_length;
            bool negative_value = ((double) value < 0);

            if (negative_value || prepend_plus_if_positive || prepend_blank_if_positive) {
                char sign = negative_value ? '-' : (prepend_plus_if_positive ? '+' : ' ');
                if (callback(ctx, &sign, 1) != 1)
                    return -1;
                total_length++;
            }

            // Apply field width padding
            if (abs_field_width > total_length && !field_width_is_negative) {
                for (size_t i = total_length; i < abs_field_width; i++) {
                    if (callback(ctx, " ", 1) != 1)
                        return -1;
                    written++;
                }
            }

            // Print the float
            if (callback(ctx, float_buffer, float_length) != float_length)
                return -1;
            written += float_length;

            // Right padding if negative field width
            if (field_width_is_negative && abs_field_width > total_length) {
                for (size_t i = total_length; i < abs_field_width; i++) {
                    if (callback(ctx, " ", 1) != 1)
                        return -1;
                    written++;
                }
            }

        } else if (*format == 'c' && (format++, true)) {
            char c;
            if (length == LENGTH_DEFAULT)
                c = (char) va_arg(parameters, int);
            else if (length == LENGTH_LONG) {
                // TODO: Implement wide character printing.
                (void) va_arg(parameters, uint32_t);
                goto unsupported_conversion;
            } else
                goto incomprehensible_conversion;

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
        } else if (*format == 'm' || *format == 's') {
            char conversion = *format++;

            const char* string;
            if (length == LENGTH_DEFAULT)
                string = va_arg(parameters, const char*);
            else if (length == LENGTH_LONG) {
                // TODO: Implement wide character string printing.
                (void) va_arg(parameters, const int*);
                goto unsupported_conversion;
            } else
                goto incomprehensible_conversion;

            if (conversion == 's' && !string)
                string = "(null)";

            size_t string_length = 0;
            for (size_t i = 0; i < precision && string[i]; i++)
                string_length++;

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
        } else if (*format == 'n' && (format++, true)) {
            if (length == LENGTH_SHORT_SHORT)
                *va_arg(parameters, signed char*) = (signed char) written;
            else if (length == LENGTH_SHORT)
                *va_arg(parameters, short*) = (short) written;
            else if (length == LENGTH_DEFAULT)
                *va_arg(parameters, int*) = (int) written;
            else if (length == LENGTH_LONG)
                *va_arg(parameters, long*) = (long) written;
            else if (length == LENGTH_LONG_LONG)
                *va_arg(parameters, long long*) = (long long) written;
            else if (length == LENGTH_INTMAX_T)
                *va_arg(parameters, uintmax_t*) = (uintmax_t) written;
            else if (length == LENGTH_SIZE_T)
                *va_arg(parameters, size_t*) = (size_t) written;
            else if (length == LENGTH_PTRDIFF_T)
                *va_arg(parameters, ptrdiff_t*) = (ptrdiff_t) written;
            else
                goto incomprehensible_conversion;
        } else
            goto incomprehensible_conversion;
    }

    if (INT32_MAX < written)
        return -1;

    return written;
}
