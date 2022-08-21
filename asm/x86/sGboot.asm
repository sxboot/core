;
; Copyright (C) 2022 user94729 and contributors
;
; This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
; If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
;

bits 16
org 0x7e00

; VERSION 3C

jmp		main16

%define S1BOOT_LOCATION 0x1000

bpbOEM				db	"TSXBOOT "
bpbBytesPerSector	dw	512
bpbSecsPerCluster	db	1
bpbReservedSectors	dw	0x81
bpbNumOfFATs		db	2
bpbRootEntries		dw	512
bpbSectors			dw	0x8000
bpbDrive			db	0xf8
bpbSectorsPerFAT	dw	0x007f
bpbSectorsPerTrack	dw	0x003f
bpbHeadsPerCylinder	dw	0x00ff
bpbHiddenSectors	dd	0x00008000
bpbSectorsL			dd	0

bpbDriveNumber		db	0x80
bpbReserved			db	0
bpbBootSignature	db	0x29
bpbVolumeID			dd	0x9ed5acc1
bpbVolumeLabel		db	"TSXBOOTPART"
bpbFileSystem		db	"FAT16   "

bootDrive	db	0

msgPre		db	"[sG] ", 0
msgLoad		db	"Loading s1boot", 0
msgSuccess	db	"success", 0
msgFail		db	"Error, press any key to reboot", 0

signature	db	"TSXBOOT3"

print:
	lodsb
	or		al, al
	jz		.complete
	mov		ah, 0x0e
	int		0x10
	jmp		print
	.complete:
	ret

println:
	push	ax
	push	si
	mov		si, msgPre
	call	print
	pop		si
	call	print
	call	printNln
	pop		ax
	ret

printNln:
	mov		ah, 0x0e
	mov		al, 0x0d
	int		0x10
	mov		al, 0x0a
	int		0x10
	ret

printP:
	mov		ah, 0x0e
	mov		al, 0x2e
	int		0x10
	ret

fail:
	call	printNln
	mov		si, msgFail
	call	println
	mov		ah, 0x00
	int		0x16
	int		0x19

align 4

dap_start:
dap_size	db	0x10
dap_rsrvd	db	0
dap_count	dw	0
dap_buf_off	dw	0
dap_buf_seg	dw	0
dap_start_l	dd	0
dap_start_h	dd	0

readSectors16:
	;edx = start sector    cx = to read    bx = dest
	mov		DWORD[dap_start_l], edx
	mov		WORD[dap_count], cx
	mov		WORD[dap_buf_off], bx

	push	dx
	push	si
	mov		ah, 0x42
	mov		dl, BYTE[bootDrive]
	mov		si, dap_start
	int		0x13
	pop		si
	pop		dx
	jc		fail
	call	printP
	ret

main16:
	mov		ax, 0
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax

	cmp		bl, 0x54
	jne		fail

	mov		DWORD[bpbHiddenSectors], edx
	mov		BYTE[bootDrive], cl

	mov		si, msgPre
	call	print
	mov		si, msgLoad
	call	print

	call	printP

	;get extensions available
	mov		dl, BYTE[bootDrive]
	mov		ah, 0x41
	mov		bx, 0x55aa
	int		0x13
	jc		fail

	mov		edx, DWORD[bpbHiddenSectors]
	add		edx, 1
	mov		cx, 1
	mov		bx, 0x8000
	call	readSectors16

	mov		cx, 8
	mov		si, signature
	mov		di, 0x8000
	.cmp:
	lodsb
	cmp		BYTE[di], al
	jne		fail
	inc		di
	loop	.cmp

	call	printP
	;stage 1 information structure:
	;0  1  2  3  4  5  6  7  8 9 a b       c d     e f          10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f
	;\T \S \X \B \O \O \T \3 02            08      00
	;signature               start sector  length  default part
	mov		edx, DWORD[bpbHiddenSectors]
	add		edx, DWORD[0x8008]
	mov		cx, WORD[0x800c]
	mov		bx, S1BOOT_LOCATION
	call	readSectors16

	call	printNln
	mov		si, msgSuccess
	call	println
	mov		dl, BYTE[bootDrive]
	mov		cl, BYTE[0x800e]
	mov		bx, 0xca55
	jmp		S1BOOT_LOCATION


times 510 - ($-$$) db 0
dw 0xaa55
