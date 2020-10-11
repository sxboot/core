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

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <x86/x86.h>
#include <x86/pci.h>

uint32_t pci_enum(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset){
	uint32_t out = (((uint32_t) bus) << 16) | (((uint32_t) device) << 11) | (((uint32_t) function) << 8) | (((uint32_t) offset) & 0xfc) | ((uint32_t) 0x80000000);
	
	x86_outd(0xcf8, out);
	
	uint32_t data = (x86_ind(0xcfc) >> ((offset & 0x2) * 8));
	return data;
}
