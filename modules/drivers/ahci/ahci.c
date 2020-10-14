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
 * ahci.c - AHCI disk driver.
 */

#include <klibc/stdlib.h>
#include <klibc/stdint.h>
#include <klibc/stdbool.h>
#include <klibc/string.h>
#include <kernel/mmgr.h>
#include <kernel/kutil.h>
#include <kernel/errc.h>
#include <kernel/log.h>
#include <x86/pci.h>
#include "ahci.h"

static ahci_controller ahci_controllers[AHCI_MAX_HBA_COUNT];
static uint8_t ahci_hba_count = 0;

static bool ahci_initialized = false;
static bool ahci_device_mapped = false;

static ahci_rec_fis* ahci_rfis_temp = NULL;
static ahci_cmd_header* ahci_cmdh_temp = NULL;


status_t ahci_detect_hba(int maxBus, int maxSlot){
	status_t status = 0;
	for(int bus = 0; bus < maxBus; bus++)
		for(int dev = 0; dev < maxSlot; dev++){
			// vendor
			if((pci_enum(bus, dev, 0, 0) & 0xffff) != 0xffff){
				// class: 1 - mass storage, 6 - serial ata
				if((pci_enum(bus, dev, 0, 10) & 0xffff) == 0x0106){
					// bar 5 (ahci base memory register)
					ahci_controllers[ahci_hba_count].id = ahci_hba_count;
					ahci_controllers[ahci_hba_count].mem = (hba_memory*) ((size_t) pci_enum(bus, dev, 0, 0x24));
					reloc_ptr((void**) &ahci_controllers[ahci_hba_count].mem);
					status = vmmgr_map_page((size_t) (ahci_controllers[ahci_hba_count].mem), (size_t) (ahci_controllers[ahci_hba_count].mem));
					CERROR();
					ahci_controllers[ahci_hba_count].flags = 1;
					ahci_hba_count++;
					if(ahci_hba_count >= AHCI_MAX_HBA_COUNT)
						goto _end;
				}
			}
		}
	_end:
	return status;
}

ahci_controller* ahci_get_controllers(){
	return ahci_controllers;
}

uint8_t ahci_get_controller_count(){
	return ahci_hba_count;
}

uint8_t ahci_get_device_type(hba_port* port){
	if(!ahci_device_active(port))
		return HBA_DEV_NONE;
	switch(port->pxsig){
		case 0x00000101: return HBA_DEV_SATA;
		case 0xeb140101: return HBA_DEV_SATAPI;
		case 0x96690101: return HBA_DEV_PMUL;
		case 0xc33c0101: return HBA_DEV_EMB;
		default: return HBA_DEV_UNKNOWN;
	}
}

bool ahci_controller_present(uint8_t ahciNum){
	if(ahciNum >= ahci_hba_count)
		return FALSE;
	return ahci_controllers[ahciNum].flags & 1;
}

bool ahci_port_present(uint8_t ahciNum, uint8_t portNum){
	if(!ahci_controller_present(ahciNum))
		return FALSE;
	return ((ahci_controllers[ahciNum].mem->pi) >> portNum) & 1;
}

bool ahci_device_active(hba_port* port){
	return (((port->pxssts) >> 8) & 0xf) == 1 && ((port->pxssts) & 0xf) == 3;
}

bool ahci_device_present(uint8_t ahciNum, uint8_t portNum){
	if(!ahci_port_present(ahciNum, portNum))
		return FALSE;
	ahci_device* device = &ahci_controllers[ahciNum].devices[portNum];
	if((device->type == 0xff) || (device->type == 0xfe))
		return FALSE;
	if(!ahci_device_active(device->port))
		return FALSE;
	return TRUE;
}

status_t ahci_init(){
	status_t status = ahci_detect_hba(256, 32);
	CERROR();

	if(ahci_hba_count < 1)
		FERROR(11);

	for(int i = 0; i < ahci_hba_count; i++){
		status = ahci_init_hba(i);
		CERROR();
	}
	ahci_initialized = true;
	_end:
	return status;
}

status_t ahci_init_hba(uint8_t ahciNum){
	status_t status = 0;
	if(!ahci_controller_present(ahciNum))
		FERROR(12);

	hba_memory* mem = ahci_controllers[ahciNum].mem;
	// AHCI enabled
	if(!((mem->ghc>>31)&1))
		mem->ghc |= 0x80000000;

	//interrupts enabled
	if(!((mem->ghc>>1)&1))
		mem->ghc |= 0x2;

	ahci_controllers[ahciNum].maxCmd = ((ahci_controllers[ahciNum].mem->cap >> 8) & 0x1f) + 1;

	uint8_t driveNumCounter = 0;
	for(int i = 0; i < 32; i++){
		ahci_controllers[ahciNum].devices[i].port = (hba_port*) ((size_t) ahci_controllers[ahciNum].mem + 0x100 + (i * 0x80));
		reloc_ptr((void**) &ahci_controllers[ahciNum].devices[i].port);
		status = vmmgr_map_page((size_t) (ahci_controllers[ahciNum].devices[i].port), (size_t) (ahci_controllers[ahciNum].devices[i].port));
		CERROR();
		if(!ahci_port_present(ahciNum, (uint8_t) i))
			ahci_controllers[ahciNum].devices[i].type = HBA_NO_PORT;
		else
			ahci_controllers[ahciNum].devices[i].type = ahci_get_device_type(ahci_controllers[ahciNum].devices[i].port);
		if(ahci_device_present(ahciNum, i) && driveNumCounter < 0xff){
			ahci_controllers[ahciNum].devices[i].number = driveNumCounter;
			driveNumCounter++;
		}else
			ahci_controllers[ahciNum].devices[i].number = 0xff;
	}
	ahci_controllers[ahciNum].flags |= 2;
	_end:
	return status;
}

bool ahci_controller_initialized(uint8_t ahciNum){
	if(!ahci_controller_present(ahciNum))
		return FALSE;
	return (ahci_controllers[ahciNum].flags & 2) > 0;
}

status_t ahci_dma_engine_start(hba_port* port){
	status_t status = 0;
	if(!ahci_device_active(port))
		FERROR(13);
	size_t wait_timeout = arch_time();
	// wait for CR to clear
	while((((port->pxcmd) >> 15) & 1) && arch_time() - wait_timeout < 1000)
		arch_sleep(1);
	if(arch_time() - wait_timeout >= 1000)
		FERROR(14);
	// first set FRE, then ST
	(port->pxcmd) |= 0x10;
	(port->pxcmd) |= 0x1;
	_end:
	return status;
}

status_t ahci_dma_engine_stop(hba_port* port){
	status_t status = 0;
	if(!ahci_device_active(port))
		FERROR(13);
	size_t wait_timeout = arch_time();
	// clear ST
	(port->pxcmd) &= ~0x1;
	// wait for CR to clear
	while((((port->pxcmd) >> 15) & 1) && arch_time() - wait_timeout < 1000)
		arch_sleep(1);
	if(arch_time() - wait_timeout >= 1000)
		FERROR(14);
	// now allowed to clear FRE
	(port->pxcmd) &= ~0x10;
	// wait for FR to clear
	wait_timeout = arch_time();
	while((((port->pxcmd) >> 14) & 1) && arch_time() - wait_timeout < 1000)
		arch_sleep(1);
	if(arch_time() - wait_timeout >= 1000)
		FERROR(14);
	_end:
	return status;
}

status_t ahci_device_init(ahci_device* device){
	status_t status = ahci_port_map(device->port);
	CERROR();
	device->flags |= 2;
	_end:
	return status;
}

status_t ahci_port_map(hba_port* port){
	status_t status = 0;
	if(ahci_device_mapped)
		FERROR(10);
	status = ahci_dma_engine_stop(port);
	CERROR();

	port->pxfb = (uint32_t) vmmgr_get_physical((size_t) (ahci_rfis_temp));
	port->pxfbu = 0;

	port->pxclb = (uint32_t) vmmgr_get_physical((size_t) (ahci_cmdh_temp));
	port->pxclbu = 0;

	ahci_device_mapped = true;
	status = ahci_dma_engine_start(port);
	CERROR();
	_end:
	return status;
}

status_t ahci_device_reset(ahci_device* device){
	hba_port* port = device->port;
	status_t status = ahci_dma_engine_stop(port);
	CERROR();

	port->pxfb = 0;
	port->pxfbu = 0;

	port->pxclb = 0;
	port->pxclbu = 0;

	ahci_device_mapped = false;
	device->flags &= ~2;
	_end:
	return status;
}

status_t ahci_device_reset_all(){
	status_t status = 0;
	for(int ahciNum = 0; ahciNum < ahci_hba_count; ahciNum++){
		for(int i = 0; i < 32; i++){
			if(ahci_controllers[ahciNum].devices[i].flags & 0x2){
				status = ahci_device_reset(&ahci_controllers[ahciNum].devices[i]);
				CERROR();
			}
		}
	}
	_end:
	return status;
}

uint16_t ahci_get_device(uint8_t number){
	for(uint8_t ahciNum = 0; ahciNum < AHCI_MAX_HBA_COUNT; ahciNum++){
		if(!ahci_controller_present(ahciNum))
			continue;
		for(uint8_t i = 0; i < 32; i++){
			if(!ahci_device_present(ahciNum, i))continue;
			if(ahci_controllers[ahciNum].devices[i].number == number)
				return ((uint16_t) ahciNum << 8) | i;
		}
	}
	return 0xffff;
}

uint8_t ahci_cmd_next_slot(hba_port* port, uint8_t maxCmd){
	uint32_t slots = (port->pxsact | port->pxci);
	for(int i = 0; i < maxCmd; i++){
		if((slots & 1) == 0)
			return i;
		slots >>= 1;
	}
	return -1;
}

status_t ahci_device_io(uint8_t ahciNum, uint8_t portNum, uint64_t lba, uint16_t secCount, size_t mem, bool action){
	status_t status = 0;
	void* prdt = 0;
	if(!ahci_controller_initialized(ahciNum))
		FERROR(16);
	if(!ahci_device_present(ahciNum, portNum))
		FERROR(13);
	if(ahci_device_mapped){
		status = ahci_device_reset_all();
		CERROR();
	}
	ahci_device* device = &ahci_controllers[ahciNum].devices[portNum];
	status = ahci_device_init(device);
	CERROR();
	hba_port* port = device->port;
	port->pxis = (uint32_t) -1;
	uint8_t slot = ahci_cmd_next_slot(port, ahci_controllers[ahciNum].maxCmd);
	if(slot == 0xff)
		FERROR(17);

	uint16_t prdt_entries = (uint16_t) ((secCount-1)>>4)+1;
	size_t cmdTableSize = sizeof(ahci_cmd_table) + MAX(prdt_entries, 16) * sizeof(ahci_prdt);
	prdt = kmalloc(cmdTableSize);
	if(!prdt)
		FERROR(TSX_OUT_OF_MEMORY);
	memset(prdt, 0, cmdTableSize);

	ahci_cmd_header* header = ahci_cmdh_temp;
	header+=slot;
	memset(header, 0, sizeof(ahci_cmd_header));
	header->prdtl = prdt_entries;
	header->ctba0 = (uint32_t) vmmgr_get_physical((size_t) prdt);
	header->ctba_u0 = 0;
	header->flags = (sizeof(ahci_fis_h2d_reg)/4)&0x1f;
	header->flags &= ~0x40;

	ahci_cmd_table* table = prdt;

	uint16_t sec_left = secCount;
	mem = vmmgr_get_physical(mem);
	for(int i = 0; i < header->prdtl-1; i++){
		table->prdt_entry[i].dba = mem;
		table->prdt_entry[i].dbau = 0;
		table->prdt_entry[i].flags = 8191;
		table->prdt_entry[i].flags |= 0x80000000;
		mem += 8192;
		sec_left -= 16;
	}
	table->prdt_entry[header->prdtl-1].dba = mem;
	table->prdt_entry[header->prdtl-1].dbau = 0;
	table->prdt_entry[header->prdtl-1].flags = (sec_left * 512) - 1;
	table->prdt_entry[header->prdtl-1].flags |= 0x80000000;
	
	ahci_fis_h2d_reg* cmdf = (ahci_fis_h2d_reg*) (&table->cfis);
	cmdf->type = 0x27;
	cmdf->flags = 0x80;
	cmdf->cmd = action ? ATA_CMD_DMA_WRITE : ATA_CMD_DMA_READ;
	
	cmdf->lba_low = (uint16_t) lba;
	cmdf->lba_mid0 = (uint8_t) (lba >> 16);
	cmdf->lba_mid1 = (uint8_t) (lba >> 24);
	cmdf->lba_mid2 = (uint8_t) (lba >> 32);
	cmdf->lba_high = (uint8_t) (lba >> 40);
	
	cmdf->device = 0x40;
	cmdf->count = (uint16_t) secCount;
	
	size_t wait_timeout = arch_time();
	while((port->pxtfd & 0x88) && arch_time() - wait_timeout < 1000)
		arch_sleep(1);
	if(arch_time() - wait_timeout >= 1000){
		FERROR(18);
	}
	
	port->pxci = 1 << slot;
	
	wait_timeout = arch_time();
	while(1){
		if((port->pxci & (1 << slot)) == 0)
			break;
		// fatal: PxIS.HBFS, PxIS.HBDS, PxIS.IFS, or PxIS.TFES
		// non-fatal: PxIS.INFS or PxIS.OFS
		if(arch_time() - wait_timeout >= 1000)
			FERROR(18);
		if(port->pxis & 0x78000000){
			FERROR(19);
		}
		arch_sleep(1);
	}
	status = ahci_device_reset(device);
	CERROR();
	_end:
	if(prdt)
		kfree(prdt, cmdTableSize);
	return status;
}

static char* msio_driver_type = "ahci";

status_t msio_init(){
	status_t status = 0;
	if(!ahci_initialized){
		ahci_rfis_temp = kmalloc(sizeof(ahci_rec_fis));
		if(ahci_rfis_temp == NULL)
			FERROR(TSX_OUT_OF_MEMORY);
		memset((void*) ahci_rfis_temp, 0, sizeof(ahci_rec_fis));
		reloc_ptr((void**) &ahci_rfis_temp);

		ahci_cmdh_temp = kmalloc(sizeof(ahci_cmd_header) * 32);
		if(ahci_cmdh_temp == NULL)
			FERROR(TSX_OUT_OF_MEMORY);
		memset(ahci_cmdh_temp, 0, sizeof(ahci_cmd_header) * 32);
		reloc_ptr((void**) &ahci_cmdh_temp);

		status = ahci_init();
		CERROR();
	}
	_end:
	return status;
}

status_t msio_read(uint8_t number, uint64_t sector, uint16_t sectorCount, size_t dest){
	status_t status = 0;
	if(!ahci_initialized){
		status = ahci_init();
		if(status != 0)
			return status;
	}
	uint16_t device = ahci_get_device(number);
	if(device == 0xffff)
		return TSX_NO_DEVICE;
	return ahci_device_io(device >> 8, device & 0xff, sector, sectorCount, dest, 0);
}

status_t msio_write(uint8_t number, uint64_t sector, uint16_t sectorCount, size_t source){
	status_t status = 0;
	if(!ahci_initialized){
		status = ahci_init();
		if(status != 0)
			return status;
	}
	uint16_t device = ahci_get_device(number);
	if(device == 0xffff)
		return TSX_NO_DEVICE;
	return ahci_device_io(device >> 8, device & 0xff, sector, sectorCount, source, 1);
}

char* msio_get_driver_type(){
	return msio_driver_type;
}
