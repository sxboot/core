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
 * uefi.h - Contains UEFI definitions according to the UEFI specification version 2.7.
 */

#ifndef __UEFI_H__
#define __UEFI_H__



// ------------- UEFI DEFINITIONS -------------


#if ARCH_BITS == 64
#define EFIAPI __attribute__((ms_abi))
#elif ARCH_BITS == 32
#define EFIAPI
#endif


#ifndef __WCHAR_TYPE__
#define __WCHAR_TYPE__ short
#endif

typedef unsigned long long	UINT64;
typedef long long			INT64;

typedef unsigned int		UINT32;
typedef int					INT32;

typedef unsigned short		UINT16;
typedef short				INT16;
typedef unsigned char		UINT8;
typedef char				INT8;
typedef __WCHAR_TYPE__		WCHAR;

#define VOID				void

#if ARCH_BITS == 32
typedef INT32				INTN;
typedef UINT32				UINTN;

#define EFIWARN(a)			(a)
#define EFIERR(a)			(0x80000000 | a)
#define EFIERR_OEM(a)		(0xc0000000 | a)
#elif ARCH_BITS == 64
typedef INT64				INTN;
typedef UINT64				UINTN;

#define EFIWARN(a)			(a)
#define EFIERR(a)			(0x8000000000000000 | a)
#define EFIERR_OEM(a)		(0xc000000000000000 | a)
#endif

#define EFIWARN(a)                            (a)
#define EFI_ERROR(a)              (((INTN) a) < 0)

// error codes

#define EFI_SUCCESS                             0
#define EFI_LOAD_ERROR                  EFIERR(1)
#define EFI_INVALID_PARAMETER           EFIERR(2)
#define EFI_UNSUPPORTED                 EFIERR(3)
#define EFI_BAD_BUFFER_SIZE             EFIERR(4)
#define EFI_BUFFER_TOO_SMALL            EFIERR(5)
#define EFI_NOT_READY                   EFIERR(6)
#define EFI_DEVICE_ERROR                EFIERR(7)
#define EFI_WRITE_PROTECTED             EFIERR(8)
#define EFI_OUT_OF_RESOURCES            EFIERR(9)
#define EFI_VOLUME_CORRUPTED            EFIERR(10)
#define EFI_VOLUME_FULL                 EFIERR(11)
#define EFI_NO_MEDIA                    EFIERR(12)
#define EFI_MEDIA_CHANGED               EFIERR(13)
#define EFI_NOT_FOUND                   EFIERR(14)
#define EFI_ACCESS_DENIED               EFIERR(15)
#define EFI_NO_RESPONSE                 EFIERR(16)
#define EFI_NO_MAPPING                  EFIERR(17)
#define EFI_TIMEOUT                     EFIERR(18)
#define EFI_NOT_STARTED                 EFIERR(19)
#define EFI_ALREADY_STARTED             EFIERR(20)
#define EFI_ABORTED                     EFIERR(21)
#define EFI_ICMP_ERROR                  EFIERR(22)
#define EFI_TFTP_ERROR                  EFIERR(23)
#define EFI_PROTOCOL_ERROR              EFIERR(24)
#define EFI_INCOMPATIBLE_VERSION        EFIERR(25)
#define EFI_SECURITY_VIOLATION          EFIERR(26)
#define EFI_CRC_ERROR                   EFIERR(27)
#define EFI_END_OF_MEDIA                EFIERR(28)
#define EFI_END_OF_FILE                 EFIERR(31)
#define EFI_INVALID_LANGUAGE            EFIERR(32)
#define EFI_COMPROMISED_DATA            EFIERR(33)

#define EFI_WARN_UNKNOWN_GLYPH           EFIWARN(1)
#define EFI_WARN_UNKNOWN_GLYPH          EFIWARN(1)
#define EFI_WARN_DELETE_FAILURE         EFIWARN(2)
#define EFI_WARN_WRITE_FAILURE          EFIWARN(3)
#define EFI_WARN_BUFFER_TOO_SMALL       EFIWARN(4)

// def

typedef UINT16			CHAR16;
typedef UINT8			CHAR8;
typedef UINT8			BOOLEAN;
#ifndef CONST
	#define CONST const
#endif
#ifndef TRUE
	#define TRUE	((BOOLEAN) 1)
	#define FALSE	((BOOLEAN) 0)
#endif

#ifndef NULL
	#define NULL	((VOID *) 0)
#endif

typedef UINTN		EFI_STATUS;
typedef UINT64		EFI_LBA;
typedef UINTN		EFI_TPL;
typedef VOID		*EFI_HANDLE;
typedef VOID		*EFI_EVENT;


#define IN
#define OUT
#define OPTIONAL


struct _EFI_SYSTEM_TABLE;


typedef struct EFI_GUID{
	UINT32	Data1;
	UINT16	Data2;
	UINT16	Data3;
	UINT8	Data4[8];
} EFI_GUID;


#define EFI_GLOBAL_VARIABLE {0x8BE4DF61,0x93CA,0x11d2, {0xAA,0x0D,0x00,0xE0,0x98,0x03,0x2B,0x8C}}

// ----- protocols predecl -----

struct _EFI_DEVICE_PATH_PROTOCOL;
struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
struct _EFI_GRAPHICS_OUTPUT_PROTOCOL;
struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;
struct _EFI_FILE_PROTOCOL;
struct _EFI_DISK_IO_PROTOCOL;
struct _EFI_BLOCK_IO_PROTOCOL;

// ----- protocols guid -----

#define EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID {0x387477c2,0x69c7,0x11d2, {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}
#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID {0x9042a9de,0x23dc,0x4a38, {0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}}
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID {0x0964e5b22,0x6459,0x11d2, {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}
#define EFI_DISK_IO_PROTOCOL_GUID {0xCE345171,0xBA0B,0x11d2, {0x8e,0x4F,0x00,0xa0,0xc9,0x69,0x72,0x3b}}
#define EFI_BLOCK_IO_PROTOCOL_GUID {0x964e5b21,0x6459,0x11d2, {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}
#define EFI_LOADED_IMAGE_PROTOCOL_GUID {0x5B1B31A1,0x9562,0x11d2, {0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B}}
#define EFI_DEVICE_PATH_PROTOCOL_GUID {0x09576e91,0x6d3f,0x11d2, {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}


// -------------------- FUNCTION DEFINITIONS -------------------------

// ----- function definitions (boot services) -----

// event, timer, task priority

#define EVT_TIMER						   0x80000000
#define EVT_RUNTIME						 0x40000000

#define EVT_NOTIFY_WAIT					 0x00000100
#define EVT_NOTIFY_SIGNAL				   0x00000200

#define EVT_SIGNAL_EXIT_BOOT_SERVICES	   0x00000201
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE   0x60000202

typedef
VOID
(EFIAPI *EFI_EVENT_NOTIFY) (
	IN EFI_EVENT				Event,
	IN VOID						*Context
	);

typedef
EFI_STATUS
(EFIAPI *EFI_CREATE_EVENT) (
	IN UINT32					Type,
	IN EFI_TPL					NotifyTpl,
	IN EFI_EVENT_NOTIFY			NotifyFunction,
	IN VOID						*NotifyContext,
	OUT EFI_EVENT				*Event
	);

typedef
EFI_STATUS
(EFIAPI *EFI_CREATE_EVENT_EX) (
	IN UINT32					Type,
	IN EFI_TPL					NotifyTpl,
	IN EFI_EVENT_NOTIFY			NotifyFunction OPTIONAL,
	IN CONST VOID				*NotifyContext OPTIONAL,
	IN CONST EFI_GUID			*EventGroup OPTIONAL,
	OUT EFI_EVENT				*Event
	);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOSE_EVENT) (
	IN EFI_EVENT				Event
	);

typedef
EFI_STATUS
(EFIAPI *EFI_SIGNAL_EVENT) (
	IN EFI_EVENT				Event
	);

typedef 
EFI_STATUS
(EFIAPI *EFI_WAIT_FOR_EVENT) (
	IN UINTN					NumberOfEvents,
	IN EFI_EVENT				*Event,
	OUT UINTN					*Index
	);

typedef
EFI_STATUS
(EFIAPI *EFI_CHECK_EVENT) (
	IN EFI_EVENT				Event
	);


typedef enum {
	TimerCancel,
	TimerPeriodic,
	TimerRelative,
	TimerTypeMax
} EFI_TIMER_DELAY;

typedef
EFI_STATUS
(EFIAPI *EFI_SET_TIMER) (
	IN EFI_EVENT				Event,
	IN EFI_TIMER_DELAY			Type,
	IN UINT64					TriggerTime
	);


#define TPL_APPLICATION		4
#define TPL_CALLBACK		8
#define TPL_NOTIFY			16
#define TPL_HIGH_LEVEL		31

typedef
EFI_TPL
(EFIAPI *EFI_RAISE_TPL) (
	IN EFI_TPL					NewTpl
	);

typedef
VOID
(EFIAPI *EFI_RESTORE_TPL) (
	IN EFI_TPL					OldTpl
	);


// memory allocation

typedef UINT64			EFI_PHYSICAL_ADDRESS;
typedef UINT64			EFI_VIRTUAL_ADDRESS;

typedef enum{
	AllocateAnyPages,
	AllocateMaxAddress,
	AllocateAddress,
	MaxAllocateType
} EFI_ALLOCATE_TYPE;

typedef enum{
	EfiReservedMemoryType,
	EfiLoaderCode,
	EfiLoaderData,
	EfiBootServicesCode,
	EfiBootServicesData,
	EfiRuntimeServicesCode,
	EfiRuntimeServicesData,
	EfiConventionalMemory,
	EfiUnusableMemory,
	EfiACPIReclaimMemory,
	EfiACPIMemoryNVS,
	EfiMemoryMappedIO,
	EfiMemoryMappedIOPortSpace,
	EfiPalCode,
	EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef struct EFI_MEMORY_DESCRIPTOR{
	UINT32					Type;
	EFI_PHYSICAL_ADDRESS	PhysicalStart;
	EFI_VIRTUAL_ADDRESS		VirtualStart;
	UINT64					NumberOfPages;
	UINT64					Attribute;
} EFI_MEMORY_DESCRIPTOR;

#define EFI_MEMORY_UC		0x0000000000000001
#define EFI_MEMORY_WC		0x0000000000000002
#define EFI_MEMORY_WT		0x0000000000000004
#define EFI_MEMORY_WB		0x0000000000000008
#define EFI_MEMORY_UCE		0x0000000000000010
#define EFI_MEMORY_WP		0x0000000000001000
#define EFI_MEMORY_RP		0x0000000000002000
#define EFI_MEMORY_XP		0x0000000000004000
#define EFI_MEMORY_NV		0x0000000000008000
#define EFI_MEMORY_MORE_RELIABLE	0x0000000000010000
#define EFI_MEMORY_RO		0x0000000000020000
#define EFI_MEMORY_RUNTIME	0x8000000000000000

#define EFI_MEMORY_DESCRIPTOR_VERSION	1

typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_PAGES) (
	IN EFI_ALLOCATE_TYPE		Type,
	IN EFI_MEMORY_TYPE			MemoryType,
	IN UINTN					NoPages,
	OUT EFI_PHYSICAL_ADDRESS	*Memory
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FREE_PAGES) (
	IN EFI_PHYSICAL_ADDRESS		Memory,
	IN UINTN					NoPages
	);

typedef
EFI_STATUS
(EFIAPI *EFI_GET_MEMORY_MAP) (
	IN OUT UINTN				*MemoryMapSize,
	IN OUT EFI_MEMORY_DESCRIPTOR	*MemoryMap,
	OUT UINTN					*MapKey,
	OUT UINTN					*DescriptorSize,
	OUT UINT32					*DescriptorVersion
	);

typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_POOL) (
	IN EFI_MEMORY_TYPE			PoolType,
	IN UINTN					Size,
	OUT VOID					**Buffer
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FREE_POOL) (
	IN VOID						*Buffer
	);


// protocol handlers

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL	0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL			0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL			0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER	0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER				0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE				0x00000020

typedef enum{
	EFI_NATIVE_INTERFACE,
	EFI_PCODE_INTERFACE
} EFI_INTERFACE_TYPE;

typedef enum{
	AllHandles,
	ByRegisterNotify,
	ByProtocol
} EFI_LOCATE_SEARCH_TYPE;

typedef struct EFI_OPEN_PROTOCOL_INFORMATION_ENTRY{
	EFI_HANDLE					AgentHandle;
	EFI_HANDLE					ControllerHandle;
	UINT32						Attributes;
	UINT32						OpenCount;
} EFI_OPEN_PROTOCOL_INFORMATION_ENTRY;

typedef
EFI_STATUS
(EFIAPI *EFI_INSTALL_PROTOCOL_INTERFACE) (
	IN OUT EFI_HANDLE			*Handle,
	IN EFI_GUID					*Protocol,
	IN EFI_INTERFACE_TYPE		InterfaceType,
	IN VOID						*Interface
	);

typedef
EFI_STATUS
(EFIAPI *EFI_UNINSTALL_PROTOCOL_INTERFACE) (
	IN EFI_HANDLE				Handle,
	IN EFI_GUID					*Protocol,
	IN VOID						*Interface
	);

typedef
EFI_STATUS
(EFIAPI *EFI_REINSTALL_PROTOCOL_INTERFACE) (
	IN EFI_HANDLE				Handle,
	IN EFI_GUID					*Protocol,
	IN VOID						*OldInterface,
	IN VOID						*NewInterface
	);

typedef
EFI_STATUS 
(EFIAPI *EFI_REGISTER_PROTOCOL_NOTIFY) (
	IN EFI_GUID					*Protocol,
	IN EFI_EVENT				Event,
	OUT VOID					**Registration
	);

typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_HANDLE) (
	IN EFI_LOCATE_SEARCH_TYPE	SearchType,
	IN EFI_GUID					*Protocol OPTIONAL,
	IN VOID						*SearchKey OPTIONAL,
	IN OUT UINTN				*BufferSize,
	OUT EFI_HANDLE				*Buffer
	);

typedef
EFI_STATUS
(EFIAPI *EFI_HANDLE_PROTOCOL) (
	IN EFI_HANDLE				Handle,
	IN EFI_GUID					*Protocol,
	OUT VOID					**Interface
	);

typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_DEVICE_PATH) (
	IN EFI_GUID					*Protocol,
	IN OUT struct _EFI_DEVICE_PATH_PROTOCOL		**DevicePath,
	OUT EFI_HANDLE				*Device
	);

typedef
EFI_STATUS
(EFIAPI *EFI_OPEN_PROTOCOL) (
	IN EFI_HANDLE				Handle,
	IN EFI_GUID					*Protocol,
	OUT VOID					**Interface OPTIONAL,
	IN EFI_HANDLE				AgentHandle,
	IN EFI_HANDLE				ControllerHandle,
	IN UINT32					Attributes
	);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOSE_PROTOCOL) (
	IN EFI_HANDLE				Handle,
	IN EFI_GUID					*Protocol,
	IN EFI_HANDLE				AgentHandle,
	IN EFI_HANDLE				ControllerHandle
	);

typedef
EFI_STATUS
(EFIAPI *EFI_OPEN_PROTOCOL_INFORMATION) (
	IN EFI_HANDLE				Handle,
	IN EFI_GUID					*Protocol,
	OUT EFI_OPEN_PROTOCOL_INFORMATION_ENTRY	**EntryBuffer,
	OUT UINTN					*EntryCount
	);

typedef
EFI_STATUS
(EFIAPI *EFI_CONNECT_CONTROLLER) (
	IN EFI_HANDLE				ControllerHandle,
	IN EFI_HANDLE				*DriverImageHandle OPTIONAL,
	IN struct _EFI_DEVICE_PATH_PROTOCOL			*RemainingDevicePath OPTIONAL,
	IN BOOLEAN					Recursive
	);

typedef
EFI_STATUS
(EFIAPI *EFI_DISCONNECT_CONTROLLER) (
	IN EFI_HANDLE				ControllerHandle,
	IN EFI_HANDLE				DriverImageHandle OPTIONAL,
	IN EFI_HANDLE				ChildHandle OPTIONAL
	);

typedef
EFI_STATUS
(EFIAPI *EFI_PROTOCOLS_PER_HANDLE) (
	IN EFI_HANDLE				Handle,
	OUT EFI_GUID				***ProtocolBuffer,
	OUT UINTN					*ProtocolBufferCount
	);

typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_HANDLE_BUFFER) (
	IN EFI_LOCATE_SEARCH_TYPE	SearchType,
	IN EFI_GUID					*Protocol OPTIONAL,
	IN VOID						*SearchKey OPTIONAL,
	IN OUT UINTN				*NoHandles,
	OUT EFI_HANDLE				**Buffer
	);

typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_PROTOCOL) (
	IN EFI_GUID					*Protocol,
	IN VOID						*Registration OPTIONAL,
	OUT VOID					**Interface
	);

typedef
EFI_STATUS
(EFIAPI *EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES) (
	IN OUT EFI_HANDLE			*Handle,
	...
	);

typedef
EFI_STATUS
(EFIAPI *EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES) (
	IN OUT EFI_HANDLE			Handle,
	...
	);


// image services

typedef 
EFI_STATUS
(EFIAPI *EFI_IMAGE_LOAD) (
	IN BOOLEAN					BootPolicy,
	IN EFI_HANDLE				ParentImageHandle,
	IN struct _EFI_DEVICE_PATH_PROTOCOL			*FilePath,
	IN VOID						*SourceBuffer OPTIONAL,
	IN UINTN						SourceSize,
	OUT EFI_HANDLE				*ImageHandle
	);

typedef 
EFI_STATUS
(EFIAPI *EFI_IMAGE_START) (
	IN EFI_HANDLE				ImageHandle,
	OUT UINTN					*ExitDataSize,
	OUT CHAR16					**ExitData OPTIONAL
	);

typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_UNLOAD) (
	IN EFI_HANDLE				ImageHandle
	);

typedef 
EFI_STATUS
(EFIAPI *EFI_IMAGE_ENTRY_POINT) (
	IN EFI_HANDLE				ImageHandle,
	IN struct _EFI_SYSTEM_TABLE	*SystemTable
	);

typedef
EFI_STATUS
(EFIAPI *EFI_EXIT) (
	IN EFI_HANDLE				ImageHandle,
	IN EFI_STATUS				ExitStatus,
	IN UINTN					ExitDataSize,
	IN CHAR16					*ExitData OPTIONAL
	);

typedef
EFI_STATUS
(EFIAPI *EFI_EXIT_BOOT_SERVICES) (
	IN EFI_HANDLE				ImageHandle,
	IN UINTN					MapKey
	);

typedef
EFI_STATUS
(EFIAPI *EFI_SET_WATCHDOG_TIMER) (
	IN UINTN					Timeout,
	IN UINT64					WatchdogCode,
	IN UINTN					DataSize,
	IN CHAR16					*WatchdogData OPTIONAL
	);

typedef
EFI_STATUS
(EFIAPI *EFI_STALL) (
	IN UINTN					Microseconds
	);

typedef
VOID
(EFIAPI *EFI_COPY_MEM) (
	IN VOID						*Destination,
	IN VOID						*Source,
	IN UINTN					Length
	);

typedef
VOID
(EFIAPI *EFI_SET_MEM) (
	IN VOID						*Buffer,
	IN UINTN					Size,
	IN UINT8					Value
	);

typedef
EFI_STATUS
(EFIAPI *EFI_GET_NEXT_MONOTONIC_COUNT) (
	OUT UINT64					*Count
	);

typedef
EFI_STATUS
(EFIAPI *EFI_INSTALL_CONFIGURATION_TABLE) (
	IN EFI_GUID					*Guid,
	IN VOID						*Table
	);

typedef
EFI_STATUS
(EFIAPI *EFI_CALCULATE_CRC32) (
	IN VOID						*Data,
	IN UINTN					DataSize,
	OUT UINT32					*Crc32
	);


// ----- function definitions (runtime services) -----

// variable services

#define EFI_VARIABLE_NON_VOLATILE        0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS     0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS       0x00000004
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD   0x00000008
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS 0x00000010
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS 0x00000020
#define EFI_VARIABLE_APPEND_WRITE        0x00000040
#define EFI_VARIABLE_ENHANCED_AUTHENTICATED_ACCESS 0x00000080

#define EFI_VARIABLE_AUTHENTICATION_3_CERT_ID_SHA256 1
typedef struct EFI_VARIABLE_AUTHENTICATION_3_CERT_ID{
	UINT8		Type;
	UINT32		IdSize;
	//UINT8		Id[IdSize];
} EFI_VARIABLE_AUTHENTICATION_3_CERT_ID; 

typedef
EFI_STATUS
(EFIAPI *EFI_GET_VARIABLE) (
	IN CHAR16					*VariableName,
	IN EFI_GUID					*VendorGuid,
	OUT UINT32					*Attributes OPTIONAL,
	IN OUT UINTN				*DataSize,
	OUT VOID					*Data OPTIONAL
	);

typedef
EFI_STATUS
(EFIAPI *EFI_GET_NEXT_VARIABLE_NAME) (
	IN OUT UINTN				*VariableNameSize,
	IN OUT CHAR16				*VariableName,
	IN OUT EFI_GUID				*VendorGuid
	);


typedef
EFI_STATUS
(EFIAPI *EFI_SET_VARIABLE) (
	IN CHAR16					*VariableName,
	IN EFI_GUID					*VendorGuid,
	IN UINT32					Attributes,
	IN UINTN					DataSize,
	IN VOID						*Data
	);

typedef
EFI_STATUS
(EFIAPI *EFI_QUERY_VARIABLE_INFO) (
	IN  UINT32					Attributes,
	OUT UINT64					*MaximumVariableStorageSize,
	OUT UINT64					*RemainingVariableStorageSize,
	OUT UINT64					*MaximumVariableSize
	);


// time services

typedef struct EFI_TIME{
	UINT16		Year;
	UINT8		Month;
	UINT8		Day;
	UINT8		Hour;
	UINT8		Minute;
	UINT8		Second;
	UINT8		Pad1;
	UINT32		Nanosecond;
	INT16		TimeZone;
	UINT8		Daylight;
	UINT8		Pad2;
} EFI_TIME;

typedef struct EFI_TIME_CAPABILITIES{
	UINT32		Resolution;
	UINT32		Accuracy;
	BOOLEAN		SetsToZero;
} EFI_TIME_CAPABILITIES;

typedef
EFI_STATUS
(EFIAPI *EFI_GET_TIME) (
	OUT EFI_TIME				*Time,
	OUT EFI_TIME_CAPABILITIES	*Capabilities OPTIONAL
	);

typedef
EFI_STATUS
(EFIAPI *EFI_SET_TIME) (
	IN EFI_TIME					*Time
	);

typedef
EFI_STATUS
(EFIAPI *EFI_GET_WAKEUP_TIME) (
	OUT BOOLEAN					*Enabled,
	OUT BOOLEAN					*Pending,
	OUT EFI_TIME				*Time
	);

typedef
EFI_STATUS
(EFIAPI *EFI_SET_WAKEUP_TIME) (
	IN BOOLEAN					Enable,
	IN EFI_TIME					*Time OPTIONAL
	);


// virtual memory services

typedef 
EFI_STATUS
(EFIAPI *EFI_SET_VIRTUAL_ADDRESS_MAP) (
	IN UINTN					MemoryMapSize,
	IN UINTN					DescriptorSize,
	IN UINT32					DescriptorVersion,
	IN EFI_MEMORY_DESCRIPTOR	*VirtualMap
	);

typedef 
EFI_STATUS
(EFIAPI *EFI_CONVERT_POINTER) (
	IN UINTN					DebugDisposition,
	IN OUT VOID					**Address
	);


// misc

typedef enum{
	EfiResetCold,
	EfiResetWarm,
	EfiResetShutdown,
	EfiResetPlatformSpecific
} EFI_RESET_TYPE;

typedef struct EFI_CAPSULE_BLOCK_DESCRIPTOR{
	UINT64						Length;
	union {
		EFI_PHYSICAL_ADDRESS	DataBlock;
	   EFI_PHYSICAL_ADDRESS		ContinuationPointer;
	} Union;
} EFI_CAPSULE_BLOCK_DESCRIPTOR;

typedef struct{
	EFI_GUID					CapsuleGuid;
	UINT32						HeaderSize;
	UINT32						Flags;
	UINT32						CapsuleImageSize;
} EFI_CAPSULE_HEADER;

#define CAPSULE_FLAGS_PERSIST_ACROSS_RESET	0x00010000
#define CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE   0x00020000
#define CAPSULE_FLAGS_INITIATE_RESET		  0x00040000

typedef
EFI_STATUS
(EFIAPI *EFI_RESET_SYSTEM) (
	IN EFI_RESET_TYPE			ResetType,
	IN EFI_STATUS				ResetStatus,
	IN UINTN					DataSize,
	IN CHAR16					*ResetData OPTIONAL
	);

typedef
EFI_STATUS
(EFIAPI *EFI_GET_NEXT_HIGH_MONOTONIC_COUNT) (
	OUT UINT32					*HighCount
	);

typedef
EFI_STATUS
(EFIAPI *EFI_UPDATE_CAPSULE) (
	IN EFI_CAPSULE_HEADER		**CapsuleHeaderArray,
	IN UINTN					CapsuleCount,
	IN EFI_PHYSICAL_ADDRESS		ScatterGatherList OPTIONAL
	);

typedef
EFI_STATUS
(EFIAPI *EFI_QUERY_CAPSULE_CAPABILITIES) (
	IN  EFI_CAPSULE_HEADER		**CapsuleHeaderArray,
	IN  UINTN					CapsuleCount,
	OUT UINT64					*MaximumCapsuleSize,
	OUT EFI_RESET_TYPE			*ResetType
	);


// ----- function definitions (other) -----

// text io

typedef struct{
	UINT16						ScanCode;
	CHAR16						UnicodeChar;
} EFI_INPUT_KEY;

typedef struct{
	INT32						MaxMode;
	INT32						Mode;
	INT32						Attribute;
	INT32						CursorColumn;
	INT32						CursorRow;
	BOOLEAN						CursorVisible;
} SIMPLE_TEXT_OUTPUT_MODE;

typedef
EFI_STATUS
(EFIAPI *EFI_INPUT_RESET) (
	IN struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL	*This,
	IN BOOLEAN					ExtendedVerification
	);

typedef
EFI_STATUS
(EFIAPI *EFI_INPUT_READ_KEY) (
	IN struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
	OUT EFI_INPUT_KEY			*Key
	);

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_RESET) (
	IN struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
	IN BOOLEAN					ExtendedVerification
	);

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_STRING) (
	IN struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
	IN CHAR16					*String
	);

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_TEST_STRING) (
	IN struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
	IN CHAR16					*String
	);

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_QUERY_MODE) (
	IN struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
	IN UINTN					ModeNumber,
	OUT UINTN					*Columns,
	OUT UINTN					*Rows
	);

typedef
EFI_STATUS
(* EFIAPI EFI_TEXT_SET_MODE) (
	IN struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
	IN UINTN					ModeNumber
	);

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_SET_ATTRIBUTE) (
	IN struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
	IN UINTN					Attribute
	);

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_CLEAR_SCREEN) (
	IN struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This
	);

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_SET_CURSOR_POSITION) (
	IN struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
	IN UINTN					Column,
	IN UINTN					Row
	);

typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_ENABLE_CURSOR) (
	IN struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
	IN BOOLEAN					Visible
	);


// graphics io

typedef enum{
	PixelRedGreenBlueReserved8BitPerColor,
	PixelBlueGreenRedReserved8BitPerColor,
	PixelBitMask,
	PixelBltOnly,
	PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef enum{
	EfiBltVideoFill,
	EfiBltVideoToBltBuffer,
	EfiBltBufferToVideo,
	EfiBltVideoToVideo,
	EfiGraphicsOutputBltOperationMax
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

typedef struct{
	UINT32						RedMask;
	UINT32						GreenMask;
	UINT32						BlueMask;
	UINT32						ReservedMask;
} EFI_PIXEL_BITMASK;

typedef struct{
	UINT32						Version;
	UINT32						HorizontalResolution;
	UINT32						VerticalResolution;
	EFI_GRAPHICS_PIXEL_FORMAT	PixelFormat;
	EFI_PIXEL_BITMASK			PixelInformation;
	UINT32						PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct{
	UINT32						MaxMode;
	UINT32						Mode;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION	*Info;
	UINTN						SizeOfInfo;
	EFI_PHYSICAL_ADDRESS		FrameBufferBase;
	UINTN						FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct{
	UINT8						Blue;
	UINT8						Green;
	UINT8						Red;
	UINT8						Reserved;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE) (
	IN struct _EFI_GRAPHICS_OUTPUT_PROTOCOL	*This,
	IN UINT32					ModeNumber,
	OUT UINTN					*SizeOfInfo,
	OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION	**Info
	);

typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE) (
	IN struct _EFI_GRAPHICS_OUTPUT_PROTOCOL	*This,
	IN UINT32					ModeNumber
	);

typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT) (
	IN struct _EFI_GRAPHICS_OUTPUT_PROTOCOL	*This,
	IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL	*BltBuffer,
	OPTIONAL IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION	BltOperation,
	IN UINTN					SourceX,
	IN UINTN					SourceY,
	IN UINTN					DestinationX,
	IN UINTN					DestinationY,
	IN UINTN					Width,
	IN UINTN					Height,
	IN UINTN					Delta OPTIONAL
	);


// "simple" fs

typedef struct{
	EFI_EVENT   Event;
	EFI_STATUS   Status;
	UINTN     BufferSize;
	VOID      *Buffer;
} EFI_FILE_IO_TOKEN; 

typedef
EFI_STATUS
(EFIAPI *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME) (
	IN struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL	*This,
	OUT struct _EFI_FILE_PROTOCOL		**Root
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_OPEN) (
	IN struct _EFI_FILE_PROTOCOL	*This,
	OUT struct _EFI_FILE_PROTOCOL	**NewHandle,
	IN CHAR16					*FileName,
	IN UINT64					OpenMode,
	IN UINT64					Attributes
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_CLOSE) (
	IN struct _EFI_FILE_PROTOCOL	*This
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_DELETE) (
	IN struct _EFI_FILE_PROTOCOL	*This
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_READ) (
	IN struct _EFI_FILE_PROTOCOL	*This,
	IN OUT UINTN				*BufferSize,
	OUT VOID					*Buffer
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_WRITE) (
	IN struct _EFI_FILE_PROTOCOL	*This,
	IN OUT UINTN				*BufferSize,
	IN VOID						*Buffer
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_GET_POSITION) (
	IN struct _EFI_FILE_PROTOCOL	*This,
	OUT UINT64					*Position
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_SET_POSITION) (
	IN struct _EFI_FILE_PROTOCOL	*This,
	IN UINT64					Position
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_GET_INFO) (
	IN struct _EFI_FILE_PROTOCOL	*This,
	IN EFI_GUID					*InformationType,
	IN OUT UINTN				*BufferSize,
	OUT VOID					*Buffer
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_SET_INFO) (
	IN struct _EFI_FILE_PROTOCOL	*This,
	IN EFI_GUID					*InformationType,
	IN UINTN					BufferSize,
	IN VOID						*Buffer
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_FLUSH) (
	IN struct _EFI_FILE_PROTOCOL	*This
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_OPEN_EX) (
	IN struct _EFI_FILE_PROTOCOL	*This,
	OUT struct _EFI_FILE_PROTOCOL	**NewHandle,
	IN CHAR16					*FileName,
	IN UINT64					OpenMode,
	IN UINT64					Attributes,
	IN OUT EFI_FILE_IO_TOKEN	*Token
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_READ_EX) (
	IN struct _EFI_FILE_PROTOCOL	*This,
	IN OUT EFI_FILE_IO_TOKEN	*Token
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_WRITE_EX) (
	IN struct _EFI_FILE_PROTOCOL	*This,
	IN OUT EFI_FILE_IO_TOKEN	*Token
	);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_FLUSH_EX) (
	IN struct _EFI_FILE_PROTOCOL	*This,
	IN OUT EFI_FILE_IO_TOKEN	*Token
	);


// disk io

typedef
EFI_STATUS
(EFIAPI *EFI_DISK_READ) (
	IN struct _EFI_DISK_IO_PROTOCOL *This,
	IN UINT32					MediaId,
	IN UINT64					Offset,
	IN UINTN					BufferSize,
	OUT VOID					*Buffer
	);

typedef
EFI_STATUS(EFIAPI *EFI_DISK_WRITE) (
	IN struct _EFI_DISK_IO_PROTOCOL *This,
	IN UINT32					MediaId,
	IN UINT64					Offset,
	IN UINTN					BufferSize,
	IN VOID						*Buffer
	);


// block io

typedef struct{
	UINT32						MediaId;
	BOOLEAN						RemovableMedia;
	BOOLEAN						MediaPresent;
	BOOLEAN						LogicalPartition;
	BOOLEAN						ReadOnly;
	BOOLEAN						WriteCaching;
	UINT32						BlockSize;
	UINT32						IoAlign;
	EFI_LBA						LastBlock;
	EFI_LBA						LowestAlignedLba; //added in Revision 2
	UINT32						LogicalBlocksPerPhysicalBlock; //added in Revision 2
	UINT32						OptimalTransferLengthGranularity;// added in Revision 3
} EFI_BLOCK_IO_MEDIA;

typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_RESET) (
	IN struct _EFI_BLOCK_IO_PROTOCOL	*This,
	IN BOOLEAN					ExtendedVerification
	);

typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_READ) (
	IN struct _EFI_BLOCK_IO_PROTOCOL	*This,
	IN UINT32					MediaId,
	IN EFI_LBA					LBA,
	IN UINTN					BufferSize,
	OUT VOID					*Buffer
	);

typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_WRITE) (
	IN struct _EFI_BLOCK_IO_PROTOCOL	*This,
	IN UINT32					MediaId,
	IN EFI_LBA					LBA,
	IN UINTN					BufferSize,
	IN VOID						*Buffer
	);

typedef
EFI_STATUS
(EFIAPI *EFI_BLOCK_FLUSH) (
	IN struct _EFI_BLOCK_IO_PROTOCOL	*This
	);


// loaded image

typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_UNLOAD) (
	IN EFI_HANDLE  ImageHandle
	);





// -------------------- STRUCTS -------------------------

// ----- protocol structs -----

typedef struct _EFI_DEVICE_PATH_PROTOCOL{
	UINT8 Type;
	UINT8 SubType;
	UINT8 Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL{
	EFI_INPUT_RESET				Reset;
	EFI_INPUT_READ_KEY			ReadKeyStroke;
	EFI_EVENT					WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL{
	EFI_TEXT_RESET				Reset;
	EFI_TEXT_STRING				OutputString;
	EFI_TEXT_TEST_STRING		TestString;
	EFI_TEXT_QUERY_MODE			QueryMode;
	EFI_TEXT_SET_MODE			SetMode;
	EFI_TEXT_SET_ATTRIBUTE		SetAttribute;
	EFI_TEXT_CLEAR_SCREEN		ClearScreen;
	EFI_TEXT_SET_CURSOR_POSITION	SetCursorPosition;
	EFI_TEXT_ENABLE_CURSOR		EnableCursor;
	SIMPLE_TEXT_OUTPUT_MODE		*Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;


typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL{
	EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE	QueryMode;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE	SetMode;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT	Blt;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE	*Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;


typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL{
	UINT64						Revision;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME	OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef struct _EFI_FILE_PROTOCOL{
	UINT64						Revision;
	EFI_FILE_OPEN				Open;
	EFI_FILE_CLOSE				Close;
	EFI_FILE_DELETE				Delete;
	EFI_FILE_READ				Read;
	EFI_FILE_WRITE				Write;
	EFI_FILE_GET_POSITION		GetPosition;
	EFI_FILE_SET_POSITION		SetPosition;
	EFI_FILE_GET_INFO			GetInfo;
	EFI_FILE_SET_INFO			SetInfo;
	EFI_FILE_FLUSH				Flush;
	EFI_FILE_OPEN_EX			OpenEx; // Added for revision 2
	EFI_FILE_READ_EX			ReadEx; // Added for revision 2
	EFI_FILE_WRITE_EX			WriteEx; // Added for revision 2
	EFI_FILE_FLUSH_EX			FlushEx; // Added for revision 2
} EFI_FILE_PROTOCOL;


typedef struct _EFI_DISK_IO_PROTOCOL{
	UINT64						Revision;
	EFI_DISK_READ				ReadDisk;
	EFI_DISK_WRITE				WriteDisk;
} EFI_DISK_IO_PROTOCOL;


typedef struct _EFI_BLOCK_IO_PROTOCOL{
	UINT64						Revision;
	EFI_BLOCK_IO_MEDIA			*Media;
	EFI_BLOCK_RESET				Reset;
	EFI_BLOCK_READ				ReadBlocks;
	EFI_BLOCK_WRITE				WriteBlocks;
	EFI_BLOCK_FLUSH				FlushBlocks;
} EFI_BLOCK_IO_PROTOCOL;


typedef struct{
	UINT32						Revision;
	EFI_HANDLE					ParentHandle;
	struct _EFI_SYSTEM_TABLE	*SystemTable;
	// Source location of the image
	EFI_HANDLE					DeviceHandle;
	EFI_DEVICE_PATH_PROTOCOL	*FilePath;
	VOID						*Reserved;
	// Image’s load options
	UINT32						LoadOptionsSize;
	VOID						*LoadOptions;
	// Location where image was loaded
	VOID						*ImageBase;
	UINT64						ImageSize;
	EFI_MEMORY_TYPE				ImageCodeType;
	EFI_MEMORY_TYPE				ImageDataType;
	EFI_IMAGE_UNLOAD			Unload;
} EFI_LOADED_IMAGE_PROTOCOL;



// ----- table structs -----

#define EFI_2_70_SYSTEM_TABLE_REVISION ((2<<16) | (70))
#define EFI_2_60_SYSTEM_TABLE_REVISION ((2<<16) | (60))
#define EFI_2_50_SYSTEM_TABLE_REVISION ((2<<16) | (50))
#define EFI_2_40_SYSTEM_TABLE_REVISION ((2<<16) | (40))
#define EFI_2_31_SYSTEM_TABLE_REVISION ((2<<16) | (31))
#define EFI_2_30_SYSTEM_TABLE_REVISION ((2<<16) | (30))
#define EFI_2_20_SYSTEM_TABLE_REVISION ((2<<16) | (20))
#define EFI_2_10_SYSTEM_TABLE_REVISION ((2<<16) | (10))
#define EFI_2_00_SYSTEM_TABLE_REVISION ((2<<16) | (00))
#define EFI_1_10_SYSTEM_TABLE_REVISION ((1<<16) | (10))
#define EFI_1_02_SYSTEM_TABLE_REVISION ((1<<16) | (02))
#define EFI_SPECIFICATION_VERSION		EFI_2_70_SYSTEM_TABLE_REVISION

typedef struct _EFI_TABLE_HEADER{
	UINT64							Signature;
	UINT32							Revision;
	UINT32							HeaderSize;
	UINT32							CRC32;
	UINT32							Reserved;
} EFI_TABLE_HEADER;


// UEFI System Table

#define EFI_SYSTEM_TABLE_SIGNATURE		0x5453595320494249
#define EFI_SYSTEM_TABLE_REVISION		EFI_SPECIFICATION_VERSION

typedef struct _EFI_SYSTEM_TABLE{
	EFI_TABLE_HEADER				Hdr;

	CHAR16*							FirmwareVendor;
	UINT32							FirmwareRevision;

	EFI_HANDLE						ConsoleInHandle;
	EFI_SIMPLE_TEXT_INPUT_PROTOCOL	*ConIn;

	EFI_HANDLE						ConsoleOutHandle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*ConOut;

	EFI_HANDLE						StandardErrorHandle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*StdErr;

	struct EFI_RUNTIME_SERVICES		*RuntimeServices;
	struct EFI_BOOT_SERVICES		*BootServices;

	UINTN							NumberOfTableEntries;
	struct EFI_CONFIGURATION_TABLE	*ConfigurationTable;
} EFI_SYSTEM_TABLE;


// UEFI Boot Services

#define EFI_BOOT_SERVICES_SIGNATURE		0x56524553544f4f42
#define EFI_BOOT_SERVICES_REVISION		EFI_SPECIFICATION_VERSION

typedef struct EFI_BOOT_SERVICES{

	EFI_TABLE_HEADER				Hdr;

	//
	// Task priority functions
	//
	EFI_RAISE_TPL					RaiseTPL;
	EFI_RESTORE_TPL					RestoreTPL;

	//
	// Memory functions
	//
	EFI_ALLOCATE_PAGES				AllocatePages;
	EFI_FREE_PAGES					FreePages;
	EFI_GET_MEMORY_MAP				GetMemoryMap;
	EFI_ALLOCATE_POOL				AllocatePool;
	EFI_FREE_POOL					FreePool;

	//
	// Event & timer functions
	//
	EFI_CREATE_EVENT				CreateEvent;
	EFI_SET_TIMER					SetTimer;
	EFI_WAIT_FOR_EVENT				WaitForEvent;
	EFI_SIGNAL_EVENT				SignalEvent;
	EFI_CLOSE_EVENT					CloseEvent;
	EFI_CHECK_EVENT					CheckEvent;

	//
	// Protocol handler functions
	//
	EFI_INSTALL_PROTOCOL_INTERFACE	InstallProtocolInterface;
	EFI_REINSTALL_PROTOCOL_INTERFACE	ReinstallProtocolInterface;
	EFI_UNINSTALL_PROTOCOL_INTERFACE	UninstallProtocolInterface;
	EFI_HANDLE_PROTOCOL				HandleProtocol;
	EFI_HANDLE_PROTOCOL				PCHandleProtocol;
	EFI_REGISTER_PROTOCOL_NOTIFY	RegisterProtocolNotify;
	EFI_LOCATE_HANDLE				LocateHandle;
	EFI_LOCATE_DEVICE_PATH			LocateDevicePath;
	EFI_INSTALL_CONFIGURATION_TABLE	InstallConfigurationTable;

	//
	// Image functions
	//
	EFI_IMAGE_LOAD					LoadImage;
	EFI_IMAGE_START					StartImage;
	EFI_EXIT						Exit;
	EFI_IMAGE_UNLOAD				UnloadImage;
	EFI_EXIT_BOOT_SERVICES			ExitBootServices;

	//
	// Misc functions
	//
	EFI_GET_NEXT_MONOTONIC_COUNT	GetNextMonotonicCount;
	EFI_STALL						Stall;
	EFI_SET_WATCHDOG_TIMER			SetWatchdogTimer;

	//
	// DriverSupport Services
	//
	EFI_CONNECT_CONTROLLER			ConnectController;
	EFI_DISCONNECT_CONTROLLER		DisconnectController;

	//
	// Open and Close Protocol Services
	//
	EFI_OPEN_PROTOCOL				OpenProtocol;
	EFI_CLOSE_PROTOCOL				CloseProtocol;
	EFI_OPEN_PROTOCOL_INFORMATION	OpenProtocolInformation;

	//
	// Library Services
	//
	EFI_PROTOCOLS_PER_HANDLE		ProtocolsPerHandle;
	EFI_LOCATE_HANDLE_BUFFER		LocateHandleBuffer;
	EFI_LOCATE_PROTOCOL				LocateProtocol;
	EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES	InstallMultipleProtocolInterfaces;
	EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES	UninstallMultipleProtocolInterfaces;

	//
	// 32-bit CRC Services
	//
	EFI_CALCULATE_CRC32				CalculateCrc32;

	//
	// Misc Services
	//
	EFI_COPY_MEM					CopyMem;
	EFI_SET_MEM						SetMem;
	EFI_CREATE_EVENT_EX				CreateEventEx;
} EFI_BOOT_SERVICES;


// UEFI Runtime Services

#define EFI_RUNTIME_SERVICES_SIGNATURE		0x56524553544e5552
#define EFI_RUNTIME_SERVICES_REVISION		EFI_SPECIFICATION_VERSION

typedef struct EFI_RUNTIME_SERVICES{
	EFI_TABLE_HEADER				Hdr;

	//
	// Time services
	//
	EFI_GET_TIME					GetTime;
	EFI_SET_TIME					SetTime;
	EFI_GET_WAKEUP_TIME				GetWakeupTime;
	EFI_SET_WAKEUP_TIME				SetWakeupTime;

	//
	// Virtual memory services
	//
	EFI_SET_VIRTUAL_ADDRESS_MAP		SetVirtualAddressMap;
	EFI_CONVERT_POINTER				ConvertPointer;

	//
	// Variable serviers
	//
	EFI_GET_VARIABLE				GetVariable;
	EFI_GET_NEXT_VARIABLE_NAME		GetNextVariableName;
	EFI_SET_VARIABLE				SetVariable;

	//
	// Misc
	//
	EFI_GET_NEXT_HIGH_MONOTONIC_COUNT	GetNextHighMonotonicCount;
	EFI_RESET_SYSTEM				ResetSystem;

	EFI_UPDATE_CAPSULE				UpdateCapsule;
	EFI_QUERY_CAPSULE_CAPABILITIES	QueryCapsuleCapabilities;
	EFI_QUERY_VARIABLE_INFO			QueryVariableInfo;
} EFI_RUNTIME_SERVICES;






#endif /* __UEFI_H__ */
