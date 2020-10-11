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
/*
 * s2bootdecl.h - Contains the struct passed to s2boot by s1boot.
 */


#ifndef __KERNEL_S1BOOTDECL_H__
#define __KERNEL_S1BOOTDECL_H__


#pragma pack(push,1)
typedef struct s1boot_data{
	uint8_t headerVersion;
	uint8_t bootDrive;
	uint32_t bootDriveSg;
	uint8_t bootPartN;
	uint8_t bootPartFS;
	uint8_t bootGPT;
	uint8_t bootPartCount;
	uint16_t rBufSize;
	uint16_t bInfoFlags;
	uint32_t bCylinders;
	uint32_t bHeads;
	uint32_t bSecsPTrack;
	uint64_t bSectors;
	uint16_t bBytesPSec;
	uint32_t bPEDD;
	uint16_t memLow;
	uint16_t memHigh;
	uint64_t mmapStart;
	uint16_t mmapLength;
	uint64_t s2mapStart;
	uint16_t s2mapLength;
	uint64_t s2bootAddress;
	uint32_t s2bootSize;
	uint64_t bddAddress;
	uint32_t bddSize;
	uint64_t s1bootStart;
	uint64_t s1bootEnd;
	uint64_t s2bootBase;
	uint64_t reserved;
	uint64_t framebufferBase;
	uint32_t videoWidth;
	uint32_t videoHeight;
	uint32_t videoPitch;
	uint16_t videoBpp;
	uint16_t videoMode; // 0 = graphics mode, 1 = text mode
	uint64_t memoryBase;
	status_t (*s1serviceCallback)(size_t num, size_t arg0, size_t arg1, size_t arg2);
#if ARCH_BITS == 32
	uint32_t _s1serviceCallbackExt;
#endif
} s1boot_data;
#pragma pack(pop)



#endif /* __KERNEL_S1BOOTDECL_H__ */
