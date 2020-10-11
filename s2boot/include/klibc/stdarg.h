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

#ifndef __KLIBC_STDARG_H__
#define __KLIBC_STDARG_H__


/*
typedef unsigned char* va_list;

#define	__va_size(TYPE)	((sizeof(TYPE) + sizeof(int) - 1) & ~(sizeof(int) - 1))

#define	va_start(AP, LASTARG)	(AP=((va_list)(&LASTARG) + __va_size(LASTARG)))
#define va_end(AP)	((void)0)
#define va_arg(AP, TYPE)	(AP += __va_size(TYPE), *((TYPE*)(AP - __va_size(TYPE))))

#define va_copy(dest,src)	memcpy(&(dest), &(src), sizeof(va_list))*/

typedef __builtin_va_list va_list;

#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)
#define va_copy(d,s)	__builtin_va_copy(d,s)


#endif /* __KLIBC_STDARG_H__ */
