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

#ifndef __ARCH_DYNL_H__
#define __ARCH_DYNL_H__


#define DYNL_R_x_64 1
#define DYNL_R_x_PC32 2
#define DYNL_R_x_GOT32 3
#define DYNL_R_x_PLT32 4
#define DYNL_R_x_COPY 5
#define DYNL_R_x_GLOB_DAT 6
#define DYNL_R_x_JUMP_SLOT 7
#define DYNL_R_x_RELATIVE 8
#define DYNL_R_x_GOTPCREL 9
#define DYNL_R_x_32 10
#define DYNL_R_x_32S 11
#define DYNL_R_x_16 12
#define DYNL_R_x_PC16 13
#define DYNL_R_x_8 14
#define DYNL_R_x_PC8 15
#define DYNL_R_x_PC64 24
#define DYNL_R_x_GOTOFF64 25
#define DYNL_R_x_GOTPC32 26
#define DYNL_R_x_SIZE32 32
#define DYNL_R_x_SIZE64 33

#define DYNL_R_x_SYM(info)             ((info) >> 32)
#define DYNL_R_x_TYPE(info)            ((uint32_t) (info))
#define DYNL_R_x_INFO(sym, type)       (((uint64_t) (sym) << 32) + (uint64_t) (type))


#endif /* __ARCH_DYNL_H__ */
