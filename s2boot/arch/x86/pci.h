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

#ifndef __ARCH_PCI_H__
#define __ARCH_PCI_H__


#pragma pack(push,1)
typedef struct pci_device{
	uint16_t deviceId;
	uint16_t vendorId;
	uint16_t status;
	uint16_t command;
	uint8_t classCode;
	uint8_t subclass;
	uint8_t progIF;
	uint8_t revision;
	uint8_t BIST;
	uint8_t headerType;
	uint8_t latencyTimer;
	uint8_t cacheLineSize;
} pci_device;
#pragma pack(pop)

uint32_t pci_enum(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);


#endif /* __ARCH_PCI_H__ */
