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

#ifndef __AHCI_H__
#define __AHCI_H__


#define AHCI_MAX_HBA_COUNT 10

#define HBA_NO_PORT -2
#define HBA_DEV_NONE -1
#define HBA_DEV_UNKNOWN 0
#define HBA_DEV_SATA 1
#define HBA_DEV_SATAPI 2
#define HBA_DEV_PMUL 3
#define HBA_DEV_EMB 4

#define ATA_CMD_DMA_READ 0x25
#define ATA_CMD_DMA_WRITE 0x35

#pragma pack(push,1)
typedef volatile struct hba_memory{
	uint32_t cap; // host_cap
	uint32_t ghc; // global_host_control
	uint32_t is; // interrupt_status
	uint32_t pi; // ports_implemented
	uint32_t vs; // version
	uint32_t ccc_ctl; // cmd_completion_coalescing_control
	uint32_t ccc_ports; // cmd_completion_coalescing_ports
	uint32_t em_loc; // enclosure_management_location
	uint32_t em_ctl; // enclosure_management_control
	uint32_t cap2; // host_caps_ext
	uint32_t bohc; // bios_os_handoff_control_and_status
} hba_memory;

typedef volatile struct hba_port{
	uint32_t pxclb; // port x command list base address
	uint32_t pxclbu; // .. command list base address upper 32 bits
	uint32_t pxfb; // .. fis base address
	uint32_t pxfbu; // .. fis base address upper 32 bits
	uint32_t pxis; // .. interrupt_status
	uint32_t pxie; // .. interrupt_enable
	uint32_t pxcmd; // .. command and status
	uint32_t reserved; // reserved
	uint32_t pxtfd; // .. task file data
	uint32_t pxsig; // .. signature
	uint32_t pxssts; // .. serial ata status (scr0: sstatus)
	uint32_t pxsctl; // .. serial ata control (scr2: scontrol)
	uint32_t pxserr; // .. serial ata error (scr1: serror)
	uint32_t pxsact; // .. serial ata active (scr3: sactive)
	uint32_t pxci; // .. cmd_issue
	uint32_t pxsntf; // .. serial ata notification (scr4: snotification)
	uint32_t pxfbs; // .. fis-based switching control
	uint32_t pxdevslp; // .. device sleep
	uint32_t reserved2[10]; // reserved
	uint32_t pxvs[4]; // .. vendor specific
} hba_port;

typedef struct ahci_fis_h2d_reg{
	uint8_t type; // type ( 0x27 )
	uint8_t flags; // 7 command (1)/ control (0), 6:4 reserved, 3:0 port multiplier
	uint8_t cmd; // command register
	uint8_t feature_low; // feature register lower 8 bits
	uint16_t lba_low; // lba lowest
	uint8_t lba_mid0; // lba
	uint8_t device; // device reg
	uint8_t lba_mid1; // lba
	uint8_t lba_mid2; // lba
	uint8_t lba_high; // lba highest
	uint8_t feature_high; // feature register upper 8 bits
	uint16_t count; // count
	uint8_t icc; // isochronous command completion
	uint8_t control; // control reg
	uint32_t reserved; // reserved
} ahci_fis_h2d_reg;

typedef struct ahci_fis_d2h_reg{
	uint8_t type; // type ( 0x34 )
	uint8_t flags; // 7 reserved, 6 interrupt bit, 5:4 reserved, 3:0 port multiplier
	uint8_t status; // status register
	uint8_t error; // error register
	uint16_t lba_low; // lba lowest
	uint8_t lba_mid0; // lba
	uint8_t device; // device reg
	uint8_t lba_mid1; // lba
	uint8_t lba_mid2; // lba
	uint8_t lba_high; // lba highest
	uint8_t reserved; // reserved
	uint16_t count; // count
	uint16_t reserved2; // reserved
	uint32_t reserved3; // reserved
} ahci_fis_d2h_reg;

typedef struct ahci_fis_dma_setup{
	uint8_t type; // type ( 0x41 )
	uint8_t flags; // 7 auto activate, 6 interrupt bit, 5 direction device to host (1), 4 reserved, 3:0 port multiplier
	uint16_t reserved; // reserved
	uint32_t dma_buf_low; // dma buffer low
	uint32_t dma_buf_high; // dma buffer high
	uint32_t reserved2; // reserved
	uint32_t dma_buf_offset; // dma buffer offset
	uint32_t tc; // transfer count
	uint32_t reserved3; // reserved
} ahci_fis_dma_setup;

typedef struct ahci_fis_data{
	uint8_t type; // type ( 0x46 )
	uint8_t pm; // 7:4 reserved, 3:0 port multiplier
	uint16_t reserved; // reserved
	uint32_t data[1]; // data
} ahci_fis_data;

typedef struct ahci_fis_pio_setup{
	uint8_t type; // type ( 0x5f )
	uint8_t flags; // 7 reserved, 6 interrupt bit, 5 direction device to host (1), 4 reserved, 3:0 port multiplier
	uint8_t status; // status register
	uint8_t error; // error register
	uint16_t lba_low; // lba lowest
	uint8_t lba_mid0; // lba
	uint8_t device; // device reg
	uint8_t lba_mid1; // lba
	uint8_t lba_mid2; // lba
	uint8_t lba_high; // lba highest
	uint8_t reserved; // reserved
	uint16_t count; // count
	uint8_t reserved2; // reserved
	uint8_t new_status; // new value of status reg
	uint16_t tc; // transfer count
	uint16_t reserved3; // reserved
} ahci_fis_pio_setup;

typedef struct ahci_fis_set_device_bits{
	uint8_t type; // type ( 0xa1 )
	uint8_t flags; // 7 notification bit, 6 interrupt bit, 5:4 reserved, 3:0 port multiplier
	uint8_t status; // 7 reserved, 6:4 status high, 3 reserved, 2:0 status low
	uint8_t error; // error register
	uint32_t reserved; // reserved
} ahci_fis_set_device_bits;

typedef volatile struct ahci_rec_fis{
	ahci_fis_dma_setup dma_setup;
	uint32_t reserved;
	ahci_fis_pio_setup pio_setup;
	uint32_t reserved2[3];
	ahci_fis_d2h_reg d2h_reg;
	uint32_t reserved3;
	ahci_fis_set_device_bits set_device_bits;
	uint8_t ufis[64];
	uint8_t reserved4[96];
} ahci_rec_fis;

typedef struct ahci_cmd_header{
	uint16_t flags; // 15:12 port multiplier port, 11 reserved, 10 clear busy upon r_ok, 9 BIST, 8 reset, 7 prefetchable, 6 write (1) read (0), 5 ATAPI, 04:00, cmd fis length
	uint16_t prdtl; // phys region desc table length
	uint32_t prdbc; // phys region desc byte count
	uint32_t ctba0; // 31:07 command table desc base address, 06:00 reserved
	uint32_t ctba_u0; // command table desc base address upper 32 bits
	uint32_t reserved[4]; // reserved
} ahci_cmd_header;

typedef struct ahci_prdt{
	uint32_t dba; // data base address (bit 0 reserved)
	uint32_t dbau; // data base address upper 32 bits
	uint32_t reserved; // reserved
	uint32_t flags; // 31 interrupt on completion, 30:22 reserved, 21:00 data byte count
} ahci_prdt;

typedef struct ahci_cmd_table{
	uint8_t cfis[64]; // command fis
	uint8_t acmd[16]; // ATAPI command
	uint8_t reserved[48]; // reserved
	ahci_prdt prdt_entry[1];
} ahci_cmd_table;


typedef struct ahci_device{
	uint8_t type;
	uint8_t flags; // 0 reserved, 1 mapped, 7:2 reserved
	uint8_t number;
	hba_port* port;
} ahci_device;

typedef struct ahci_controller{
	uint8_t id;
	uint8_t flags; // 0 present, 1 initialized, 7:2 reserved
	hba_memory* mem;
	uint8_t maxCmd;
	ahci_device devices[32];
} ahci_controller;
#pragma pack(pop)

status_t ahci_detect_hba(int maxBus, int maxSlot);
ahci_controller* ahci_get_controllers();
uint8_t ahci_get_controller_count();
uint8_t ahci_get_device_type(hba_port* port);
bool ahci_controller_present(uint8_t ahciNum);
bool ahci_port_present(uint8_t ahciNum, uint8_t portNum);
bool ahci_device_active(hba_port* port);
bool ahci_device_present(uint8_t ahciNum, uint8_t portNum);
status_t ahci_init();
status_t ahci_init_hba(uint8_t ahciNum);
bool ahci_controller_initialized(uint8_t ahciNum);
status_t ahci_dma_engine_start(hba_port* port);
status_t ahci_dma_engine_stop(hba_port* port);
status_t ahci_device_init(ahci_device* device);
status_t ahci_port_map(hba_port* port);
status_t ahci_device_reset(ahci_device* device);
uint8_t ahci_cmd_next_slot(hba_port* port, uint8_t maxCmd);
status_t ahci_device_io(uint8_t ahciNum, uint8_t portNum, uint64_t lba, uint16_t secCount, size_t mem, bool action);
status_t msio_init();
status_t msio_read(uint8_t number, uint64_t sector, uint16_t sectorCount, size_t dest);
status_t msio_write(uint8_t number, uint64_t sector, uint16_t sectorCount, size_t source);
char* msio_get_driver_type();


#endif /* __AHCI_H__ */
