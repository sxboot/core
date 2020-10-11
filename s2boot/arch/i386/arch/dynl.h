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


#define DYNL_R_x_32 1
#define DYNL_R_x_PC32 2
#define DYNL_R_x_GOT32 3
#define DYNL_R_x_PLT32 4
#define DYNL_R_x_COPY 5
#define DYNL_R_x_GLOB_DAT 6
#define DYNL_R_x_JUMP_SLOT 7
#define DYNL_R_x_RELATIVE 8
#define DYNL_R_x_GOTOFF 9
#define DYNL_R_x_GOTPC 10
#define DYNL_R_x_32PLT 11
#define DYNL_R_x_16 20
#define DYNL_R_x_PC16 21
#define DYNL_R_x_8 22
#define DYNL_R_x_PC8 23
#define DYNL_R_x_SIZE32 38

#define DYNL_R_x_SYM(info)             ((info) >> 8)
#define DYNL_R_x_TYPE(info)            ((uint8_t) (info))
#define DYNL_R_x_INFO(sym, type)       (((sym) << 8) + (uint8_t) (type))


#pragma pack(push,1)
typedef struct dynl_rel{
	uint32_t r_offset;
	uint32_t r_info;
} dynl_rel;

typedef struct dynl_rela{
	uint32_t r_offset;
	uint32_t r_info;
	uint32_t r_addend;
} dynl_rela;
#pragma pack(pop)


#endif /* __ARCH_DYNL_H__ */
