;
; Copyright (C) 2022 user94729 and contributors
;
; This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
; If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
;

bits 16
org 0x7c00

; VERSION 2C

jmp		main16

%define S1BOOT_LOCATION 0x1000
%define SGBOOT_LOCATION 0x7e00

bootDrive	db	0

msgPre		db	"[s0] ", 0
msgLoad		db	"Loading", 0
msgSuccess	db	"success", 0
msgSuccessG	db	"GPT", 0
msgFail		db	"Error, press key", 0

signature	db	"TSXBOOT2"

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

main16:
	mov		ax, 0
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax
	mov		sp, 0x7c00

	mov		BYTE[bootDrive], dl

	mov		ax, 0x0003
	int		0x10

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

	mov		edx, 1
	mov		cx, 1
	mov		bx, 0x7e00
	call	readSectors16

	mov		cx, 8
	mov		si, signature
	mov		di, 0x7e00
	.cmp:
	lodsb
	cmp		BYTE[di], al
	jne		loadGPT
	inc		di
	loop	.cmp

	call	printP
	;stage 1 information structure:
	;0  1  2  3  4  5  6  7  8             9       a             b  c  d  e  f  10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f
	;\T \S \X \B \O \O \T \2 02            08      00
	;signature               start sector  length  default part
	movzx	edx, BYTE[0x7e08]
	movzx	cx, BYTE[0x7e09]
	mov		bx, S1BOOT_LOCATION
	call	readSectors16

	call	printNln
	mov		si, msgSuccess
	call	println
	mov		dl, BYTE[bootDrive]
	mov		cl, BYTE[0x7e0a]
	mov		bx, 0xca54
	jmp		S1BOOT_LOCATION

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

gptSign		db	"EFI PART"
gptPartSign	db	0x8a, 0xcf, 0xe7, 0x92, 0xfd, 0xea, 0x9c, 0x6e, 0xdf, 0xd9, 0xea, 0xdf, 0xae, 0x60, 0xd6, 0xc7

loadGPT:
	mov		cx, 8
	mov		si, gptSign
	mov		di, 0x7e00
	.cmp:
	lodsb
	cmp		BYTE[di], al
	jne		fail
	inc		di
	loop	.cmp

	call	printP
	mov		edx, 2
	mov		cx, 32
	mov		bx, 0x8000
	call	readSectors16

	mov		ecx, DWORD[0x7e50]
	mov		bx, 0x8000
	.readPartEntries:
	push	cx
	push	bx
	mov		di, 0x10
	add		di, bx
	mov		cx, 16
	mov		si, gptPartSign
	.readPartEntries_comp:
	lodsb
	cmp		BYTE[di], al
	jne		.readPartEntries_comp_fail
	inc		di
	loop	.readPartEntries_comp
	pop		bx
	pop		cx
	jmp		.readPartEntries_success
	.readPartEntries_comp_fail:
	pop		bx
	pop		cx
	add		bx, WORD[0x7e54]
	loop	.readPartEntries
	jmp		fail
	.readPartEntries_success:
	call	printP
	mov		edx, DWORD[bx + 0x20]
	mov		cx, 1
	mov		bx, SGBOOT_LOCATION
	call	readSectors16
	call	printNln
	mov		BYTE[msgSuccessG - 1], 0x20
	mov		si, msgSuccess
	call	println
	mov		cl, BYTE[bootDrive]
	mov		bl, 0x54
	jmp		SGBOOT_LOCATION

times 440 - ($-$$) db 0

diskSig		db	"TOS4"
reserved2	dw	0

end_mbr_data_2:

; partition entries

times 64 - ($-end_mbr_data_2) db 0
dw 0xaa55
