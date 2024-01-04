//
// Created by praisethemoon on 03.01.24.
//

#ifndef TYPE_V_YY_H
#define TYPE_V_YY_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "utils_yy.h"

uint32_t atoi_u32_yy(const char *str, size_t len, char **endptr, atoi_result *res);
int32_t atoi_i32_yy(const char *str, size_t len, char **endptr, atoi_result *res);
uint64_t atoi_u64_yy(const char *str, size_t len, char **endptr, atoi_result *res);
int64_t atoi_i64_yy(const char *str, size_t len, char **endptr, atoi_result *res);

char *itoa_u64_yy(uint64_t val, char *buf);
char *itoa_i64_yy(int64_t val, char *buf);
char *itoa_u32_yy(uint64_t val, char *buf);
char *itoa_i32_yy(int64_t val, char *buf);


/*==============================================================================
 * Copyright (C) 2020 YaoYuan <ibireme@gmail.com>.
 * Released under the MIT license (MIT).
 *============================================================================*/


/**
 Read string as double.
 This method only accepts strings in JSON format: https://tools.ietf.org/html/rfc8259
 @param str C-string beginning with the representation of a floating-point number.
 @param endptr Ending pointer after the numerical value, or point to `str` if failed.
 @return The double number, 0.0 if failed, +/-HUGE_VAL if overflow.
 */
double yy_string_to_double(const char *str, char **endptr);

/**
 Write double to string (shortest decimal representation with null-terminator).
 @param val A double number.
 @param buf A string buffer, as least 32 bytes.
 @return The ending of this string.
 */
char *yy_double_to_string(double val, char *buf);



#endif //TYPE_V_YY_H
