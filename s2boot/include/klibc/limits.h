/*
 * Copyright (C) 2020 user94729 (https://omegazero.org/) and contributors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Covered Software is provided under this License on an "as is" basis, without warranty of any kind,
 * either expressed, implied, or statutory, including, without limitation, warranties that the Covered Software
 * is free of defects, merchantable, fit for a particular purpose or non-infringing.
 * The entire risk as to the quality and performance of the Covered Software is with You.
 */

#ifndef __KLIBC_LIMITS_H__
#define __KLIBC_LIMITS_H__

#define CHAR_BIT 8
#define SCHAR_MAX 127
#define SCHAR_MIN -SCHAR_MAX
#define UCHAR_MAX 255
#define CHAR_MIN SCHAR_MIN
#define CHAR_MAX SCHAR_MAX

#define MB_LEN_MAX 1

#define SHRT_MAX 32767
#define SHRT_MIN -SHRT_MAX
#define USHRT_MAX 65535

#define INT_MAX 2147483647
#define INT_MIN -INT_MAX
#define UINT_MAX 4294967295U

#define LONG_MAX 9223372036854775807L
#define LONG_MIN -LONG_MAX
#define ULONG_MAX 18446744073709551615UL

#define LLONG_MAX 9223372036854775807LL
#define LLONG_MIN -LLONG_MAX
#define ULLONG_MAX 18446744073709551615ULL

#if ARCH_BITS == 64
#define SIZE_MAX 0xffffffffffffffff
#else
#define SIZE_MAX 0xffffffff
#endif


#endif /* __KLIBC_LIMITS_H__ */
