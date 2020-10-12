;
; Copyright (C) 2020 user94729 (https://omegazero.org/) and contributors
;
; This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
; If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
;
; Covered Software is provided under this License on an "as is" basis, without warranty of any kind,
; either expressed, implied, or statutory, including, without limitation, warranties that the Covered Software
; is free of defects, merchantable, fit for a particular purpose or non-infringing.
; The entire risk as to the quality and performance of the Covered Software is with You.
;

bits 16
org 0x1000

; MEMORY MAP
;        0x1000+   s1boot
;0x7800 - 0x7c00   stack
;0x7c00 - 0x7dff   s0boot (mbr/vbr for mbr/chain loading)
;0x7e00 - 0x7fff   sGboot / temp/FAT entries for loading s2boot
;0x00008000 - 0x0002ffff   s2boot + bdd.so unparsed
;0x00030000 - 0x0005ffff   s2boot
;0x00060000 - 0x00063fff   page mappings
;0x00070000 - 0x00072fff   bios memory map
;0x00073000 - 0x0007ffff   s2boot map

;------------------------------------------------------------
;     MAIN STARTUP AREA
;------------------------------------------------------------

s1bootStartAddr:

jmp main16

%define s2bootFileLoad 0x8000
%define s2bootMapLocation 0x73000

align 8, db 0xcc

header_start:
headerVersion	db	49

bootDrive	db	0
bootDriveSg	dd	0
bootPartN	db	0
bootPartFS	db	0
bootGPT		db	0
bootPartC	db	4

bootDriveParams:
rBufSize	dw	0x1e
bInfoFlags	dw	0
bCylinders	dd	0
bHeads		dd	0
bSecsPTrack	dd	0
bSectors	dq	0
bBytesPSec	dw	0
bPEDD		dd	0

mem_kb1		dw	0
mem_kb2		dw	0

mmapStart	dq	0
mmapLength	dw	0

s2mapStart	dq	0
s2mapLength	dw	0

s2bAddress	dq	s2bootFileLoad
s2bSize		dd	0

bddAddress	dq	0
bddSize		dd	0

s1bootStart	dq	s1bootStartAddr
s1bootEnd	dq	s1bootEndAddr

s2bootBase	dq	0
headerReserved1	dq	0

; default x86 video mode (set by s0boot)
framebufferBase	dq	0xb8000
videoWidth	dd	80
videoHeight	dd	25
videoPitch	dd	160
videoBpp	dw	16
videoMode	dw	1

membase		dq	0

s1bootServiceCallback	dq	serviceCallback

times 16 db 0
align 8, db 0xff

mbrPartSizeKB	dd	0
mbrPartFS		db	0
mbrPartBoot		db	0

gptPartName	times 37 db 0

dec_tmp		db	"0000000000", 0

msgPre		db	"[s1] ", 0
msgPreStart	db	"Checking system", 0
msgBParamsF	db	"Failed to get boot drive parameters", 0
msgBNoExt	db	"Drive extensions not available", 0
msgMenu		db	"Press any key to enter menu", 0
msgTitle	db	"TSX Boot Manager (stage 1)", 0
msgVersion	db	"TSXBoot v4.8", 0
msgBoot		db	"1: Normal startup", 0
msgSelPart	db	"2: Select boot partition", 0
msgSelRB	db	"3: Reboot", 0
msgAuto		db	"Autoboot in ", 0
msgAutoEnd	db	" seconds", 0
msgBooting	db	"Booting from partition ", 0
msgUnsuppFS	db	"Unsupported file system: ", 0
msgNotFat16	db	"Selected partition is not FAT16", 0
msgInvPart	db	"Selected partition is invalid", 0
msgAKReboot	db	"Press any key to reboot", 0

msgDoubleF	db	"Double Fault", 0
msgInvOp	db	"Illegal Opcode", 0
msgGPF		db	"General Protection Fault", 0
msgPF		db	"Page Fault", 0
msgUnExcept	db	"Unknown error", 0
msgExcept	db	"Unrecoverable error, system halted", 0

partSelID	db	"ID", 0
partSelSize	db	"Size (KiB)", 0
partSelFS	db	"File system", 0
partSelBoot	db	"Boot", 0
partSelSeld	db	"Selected", 0
partSelGNam	db	"Name", 0
partSelPF	db	"Could not set permanent start partition", 0
partSelPF2	db	"Error ", 0

partType00	dw	partFS00 - $
partType01	dw	partFS01 - $
partType02	dw	partFS02 - $
partType03	dw	partFS03 - $
partType04	dw	partFS04 - $
partType05	dw	partFS05 - $
partType06	dw	partFS06 - $
partType07	dw	partFS07 - $
partType08	dw	partFS08 - $
partType09	dw	partFS09 - $
partType0a	dw	partFS0a - $
partType0b	dw	partFS0b - $
partType0c	dw	partFS0c - $
partType0d	dw	partFS00 - $
partType0e	dw	partFS0e - $
partType0f	dw	partFS0f - $
partType10	dw	partFS10 - $
partType11	dw	partFS11 - $
partType12	dw	partFS12 - $
partType13	dw	partFS00 - $
partType14	dw	partFS14 - $
partType15	dw	partFS15 - $
partType16	dw	partFS16 - $
partType17	dw	partFS17 - $
partType18	dw	partFS18 - $
partType19	dw	partFS19 - $
partType1a	dw	partFS1a - $
partType1b	dw	partFS1b - $
partType1c	dw	partFS1c - $
partType1d	dw	partFS1d - $
partType1e	dw	partFS1e - $
partType1f	dw	partFS1f - $

partFS00	db	"None", 0
partFS01	db	"FAT 12", 0
partFS02	db	"XENIX root", 0
partFS03	db	"XENIX usr", 0
partFS04	db	"FAT 16", 0
partFS05	db	"Extended", 0
partFS06	db	"FAT 16B", 0
partFS07	db	"Proprietary", 0
partFS08	db	"OS2/AIX/QNX", 0
partFS09	db	"AIX/QNX/OS-9", 0
partFS0a	db	"OS2 Boot", 0
partFS0b	db	"FAT 32 (CHS)", 0
partFS0c	db	"FAT 32 (LBA)", 0
partFS0e	db	"FAT 16B (LBA)", 0
partFS0f	db	"Extended (LBA)", 0
partFS10	db	"?", 0
partFS11	db	"Logical FAT 12/16", 0
partFS12	db	"?", 0
partFS14	db	"Omega FS", 0
partFS15	db	"?", 0
partFS16	db	"Hidden FAT 16B", 0
partFS17	db	"Hidden Proprietary", 0
partFS18	db	"?", 0
partFS19	db	"?", 0
partFS1a	db	"?", 0
partFS1b	db	"Hidden FAT 32", 0
partFS1c	db	"Hidden FAT 32 (LBA)", 0
partFS1d	db	"?", 0
partFS1e	db	"Hidden FAT 16 (LBA)", 0
partFS1f	db	"Hidden Extended (LBA)", 0
partFSUnk	db	"Unknown", 0

fat16Sign	db	"FAT16   "

cu_x	db	0
cu_y	db	0

align 32, db 0xcc

print16: ;pointer in esi, attributes in dh
	pusha
	.l:
	lodsb
	cmp		al, 0
	je		.c
	mov		dl, al
	call	printChar16
	jmp		.l
	.c:
	jmp		complete16

printp16: ;pointer in esi, attributes in dh
	pusha
	push	si
	mov		si, msgPre
	call	print16
	pop		si
	call	print16
	jmp		complete16

println16:
	pusha
	push	si
	mov		si, msgPre
	call	print16
	pop		si
	call	print16
	call	printNln16
	jmp		complete16

printlns16:
	pusha
	call	print16
	call	printNln16
	jmp		complete16

printChar16: ;data in dx dl-char dh-attr
	pusha
	push	es

	call	setEsReg16
	mov		di, 0
	xor		ax, ax
	xor		cx, cx
	mov		al, BYTE[cu_x]
	mov		cl, 2
	mul		cl
	add		di, ax
	mov		al, BYTE[cu_y]
	mov		cl, 160
	mul		cl
	add		di, ax

	cmp		dl, 0xa
	je		.c
	cmp		dl, 0x8
	je		.c2

	mov		WORD[es:di], dx
	inc		BYTE[cu_x]
	cmp		BYTE[cu_x], 80
	je		.c
	pop		es
	jmp		complete16
	.c:
	call	printNln16
	pop		es
	jmp		complete16
	.c2:
	call	delChar16
	pop		es
	jmp		complete16

delChar16:
	pusha
	push	es

	call	setEsReg16
	mov		di, 0
	xor		ax, ax
	xor		cx, cx
	mov		al, BYTE[cu_x]
	mov		cl, 2
	mul		cl
	add		di, ax
	mov		al, BYTE[cu_y]
	mov		cl, 160
	mul		cl
	add		di, ax
	sub		di, 2
	
	mov		WORD[es:di], 0x0720
	
	cmp		BYTE[cu_x], 0
	je		.delLine
	dec		BYTE[cu_x]
	pop		es
	jmp		complete16
	.delLine:
	mov		BYTE[cu_x], 79
	call	shiftDown16
	pop		es
	jmp		complete16

printAt16: ;pointer in esi, attributes in dh, pos in bx: bh-x bl-y
	pusha
	.l:
	lodsb
	cmp		al, 0
	je		.c
	mov		dl, al
	call	printCharAt16
	inc		bh
	jmp		.l
	.c:
	jmp		complete16

printCharAt16: ;data in dx, pos in bx: bh-x bl-y
	pusha
	push	es

	call	setEsReg16
	mov		di, 0
	xor		ax, ax
	xor		cx, cx
	mov		al, bh
	mov		cl, 2
	mul		cl
	add		di, ax
	mov		al, bl
	mov		cl, 160
	mul		cl
	add		di, ax

	mov		WORD[es:di], dx
	pop		es
	jmp		complete16

printNln16:
	pusha
	mov		BYTE[cu_x], 0
	inc		BYTE[cu_y]
	cmp		BYTE[cu_y], 25
	je		.c
	jmp		complete16
	.c:
	call	shiftUp16
	jmp		complete16

printNlnr16:
	pusha
	cmp		BYTE[cu_x], 0
	je		.r
	call	printNln16
	.r:
	jmp		complete16

delLastLine16:
	pusha
	push	es
	call	setEsReg16
	dec		BYTE[cu_y]
	mov		al, BYTE[cu_y]
	mov		cl, 160
	mul		cl
	mov		di, ax
	mov		cx, 80
	.l:
	mov		WORD[es:di], 0x0720
	add		di, 2
	loop	.l
	pop		es
	jmp		complete16

shiftUp16:
	pusha
	push	es
	call	setEsReg16
	dec		BYTE[cu_y]
	mov		di, 0
	mov		cx, 960
	.l:
	mov		edx, dWORD[es:di+160]
	mov		dWORD[es:di], edx
	add		di, 4
	loop	.l
	mov		cx, 40
	.l2:
	mov		dWORD[es:di], 0x07200720
	add		di, 4
	loop	.l2
	pop		es
	jmp		complete16

shiftDown16:
	pusha
	push	es
	call	setEsReg16
	mov		di, 0xf9c
	mov		cx, 960
	.l:
	mov		edx, dWORD[es:di-160]
	mov		dWORD[es:di], edx
	sub		di, 4
	loop	.l
	mov		cx, 40
	.l2:
	mov		dWORD[es:di], 0x07200720
	sub		di, 4
	loop	.l2
	pop		es
	jmp		complete16

clearScreen16: ;attributes in dh
	pusha
	push	es
	call	setEsReg16
	mov		di, 0
	mov		cx, 2000
	xor		ax, ax
	mov		ah, dh
	mov		al, 0x20
	.l:
	mov		WORD[es:di], ax
	add		di, 2
	loop	.l
	mov		BYTE[cu_x], 0
	mov		BYTE[cu_y], 0
	pop		es
	jmp		complete16

updateCursor16:
	pusha
	xor		ax, ax
	mov		al, BYTE[cu_y]
	mov		cl, 80
	mul		cl
	add		al, BYTE[cu_x]
	mov		bx, ax

	mov		dx, 0x3d4
	mov		al, 0xe
	out		dx, al
	mov		dx, 0x3d5
	mov		al, bh
	out		dx, al

	mov		dx, 0x3d4
	mov		al, 0xf
	out		dx, al
	mov		dx, 0x3d5
	mov		al, bl
	out		dx, al

	popa
	ret

updateCursorMemData16:
	pusha
	mov		dx, 0x3d4
	mov		al, 0xe
	out		dx, al
	inc		dx
	in		al, dx
	mov		bh, al
	mov		dx, 0x3d4
	mov		al, 0xf
	out		dx, al
	inc		dx
	in		al, dx
	mov		bl, al
	mov		ax, bx
	mov		cx, 80
	xor		dx, dx
	div		cx
	mov		BYTE[cu_x], dl
	mov		BYTE[cu_y], al
	popa
	ret

updateLoadingWheel16:
	pusha
	push	es

	call	setEsReg16
	mov		di, 0
	xor		eax, eax
	xor		cx, cx
	mov		al, BYTE[cu_x]
	mov		cl, 2
	mul		cl
	add		di, ax
	mov		al, BYTE[cu_y]
	mov		cl, 160
	mul		cl
	add		di, ax

	mov		dx, WORD[es:di]
	cmp		dl, '-'
	je		.minus
	cmp		dl, '/'
	je		.slash
	cmp		dl, 0x5c
	je		.back
	cmp		dl, '|'
	je		.line
	mov		dl, '-'
	mov		WORD[es:di], dx
	pop		es
	popa
	ret
	.minus:
	mov		dl, 0x5c
	jmp		.c
	.slash:
	mov		dl, '-'
	jmp		.c
	.back:
	mov		dl, '|'
	jmp		.c
	.line:
	mov		dl, '/'
	jmp		.c
	.c:
	mov		WORD[es:di], dx
	pop		es
	popa
	ret

setEsReg16:
	mov		ax, 0xb800
	mov		es, ax
	ret

complete16:
	call	updateCursor16
	popa
	ret

sleep16: ;unit in dx     unit = ms / 100 * 2 ??, returns 0 if time out and 1 if keyboard
	pusha
	push	dx
	xor		ah, ah
	int		0x1a
	mov		bx, dx
	pop		dx
	add		bx, dx
	.wait:
	xor		ah, ah
	int		0x1a
	cmp		bx, dx
	ja		.wait
	popa
	ret

sleepKBInt16: ;unit in dx     unit = ms / 100 * 2 ??
	pusha
	push	dx
	xor		ah, ah
	int		0x1a
	mov		bx, dx
	pop		dx
	add		bx, dx
	.wait:
	mov		ah, 1
	int		0x16
	jnz		.kb
	xor		ah, ah
	int		0x1a
	cmp		bx, dx
	ja		.wait
	popa
	mov		ax, 0
	ret
	.kb:
	popa
	mov		ax, 1
	ret

printTitle16:
	pusha
	mov		byte[cu_x], 0
	mov		byte[cu_y], 24

	xor		bx, bx
	mov		cx, 80
	mov		dx, 0xf020
	.toplc:
	call	printCharAt16
	inc		bh
	loop	.toplc

	xor		bx, bx
	mov		dh, 0x70
	mov		si, msgTitle
	call	printAt16

	mov		bh, 67
	mov		si, msgVersion
	call	printAt16
	popa
	ret

getFS16:
	push	bx
	xor		bx, bx
	cmp		dl, 0x1f
	jbe		.valid
	mov		si, partFSUnk
	pop		bx
	ret
	.valid:
	mov		bl, dl
	add		bx, bx
	add		bx, partType00
	mov		si, bx
	add		bx, WORD[si]
	mov		si, bx
	pop		bx
	ret

printLine16:
	pusha
	mov		cx, 80
	mov		bh, 0
	mov		dl, '-'
	.lp:
	call	printCharAt16
	inc		bh
	loop	.lp
	popa
	ret

clearKBBuffer16:
	push	ax
	.s:
	mov		ah, 1
	int		0x16
	jz		.e
	xor		ah, ah
	int		0x16
	jmp		.s
	.e:
	pop		ax
	ret

getDec16_32: ; number in edx
	pusha
	mov		di, dec_tmp + 9
	.reset:
	mov		BYTE[di], 0x30
	dec		di
	cmp		di, dec_tmp - 1
	jne		.reset
	mov		di, dec_tmp + 9
	mov		ebx, 10
	mov		eax, edx
	.loop:
	xor		edx, edx
	div		ebx
	add		dl, 48
	mov		BYTE[di], dl
	cmp		eax, 0
	je		.end
	dec		di
	jmp		.loop
	.end:
	popa
	mov		si, dec_tmp - 1
	.cut:
	inc		si
	cmp		BYTE[si], 0x30
	je		.cut
	cmp		si, dec_tmp + 10
	jne		.p
	mov		si, dec_tmp + 9
	.p:
	mov		ax, si
	ret

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
	;edx = start sector    cx = to read    ebx = dest
	push	ebx
	mov		DWORD[dap_start_l], edx
	mov		WORD[dap_count], cx
	mov		WORD[dap_buf_off], bx
	and		ebx, 0xffff0000
	shr		ebx, 4
	mov		WORD[dap_buf_seg], bx
	pop		ebx

	push	dx
	mov		dx, 0
	call	sleep16
	pop		dx

	push	dx
	push	si
	mov		ah, 0x42
	mov		dl, BYTE[bootDrive]
	mov		si, dap_start
	int		0x13
	pop		si
	pop		dx
	ret

sinc		db	0x0
read_start	dd	0x0

readSectors16_legacy:
	;edx = start sector    cx = to read    ebx = dest
	cmp		cx, 0
	jne		.c1
	ret
	.c1:
	mov		DWORD[read_start], edx
	push	cx
	mov		ax, WORD[bBytesPSec]
	xor		dx, dx
	mov		cx, 16
	div		cx
	mov		BYTE[sinc], al
	pop		cx
	shr		ebx, 4
	;DWORD[read_start] = start sector    cx = to read    bx = dest
	.l:
	call	updateLoadingWheel16
	push	cx
	push	bx
	push	es
	dec		cx
	mov		ax, cx
	xor		dx, dx
	mul		BYTE[sinc]
	mov		es, bx
	xor		bx, bx
	mov		dx, es
	add		dx, ax
	mov		es, dx
	add		ecx, DWORD[read_start]
	;ax = 1    ecx = mod start sector    bx = 0    es = mod dest segm

	push	di
	;add		ecx, WORD[hiddenSectors]
	mov		eax, ecx
	xor		edx, edx
	div		WORD[bSecsPTrack]
	inc		dl
	mov		cl, dl
	xor		dx, dx
	div		WORD[bHeads]
	mov		dh, dl
	mov		ch, al
	mov		dl, BYTE[bootDrive]
	mov		ah, 2
	mov		al, 1
	mov		di, 5
	.read:
	int		0x13
	jnc		.success
	dec		di
	jnz		.read
	pop		di

	pop		es
	pop		bx
	pop		cx
	jmp		.fail
	.success:
	pop		di

	pop		es
	pop		bx
	pop		cx
	loop	.l
	clc
	ret
	.fail:
	stc
	ret

main16_option2_printPartitions_getData:
	mov		ax, cx
	dec		ax
	mov		bx, 0x10
	mul		bx
	add		ax, 0x7dbe
	mov		bx, ax
	mov		dl, BYTE[bx + 4]
	mov		BYTE[mbrPartFS], dl
	mov		dl, BYTE[bx]
	mov		BYTE[mbrPartBoot], dl
	mov		eax, DWORD[bx + 12]
	mov		ebx, 2
	xor		edx, edx
	div		ebx
	mov		DWORD[mbrPartSizeKB], eax
	ret

main16:
	mov		ax, 0
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax
	mov		sp, 0x7c00

	cmp		bx, 0xca54
	je		.chkS
	mov		BYTE[bootGPT], 1
	cmp		bx, 0xca55
	je		.chkS
	cli
	hlt

	.chkS:
	mov		BYTE[bootDrive], dl
	mov		BYTE[bootPartN], cl

	call	updateCursorMemData16

	call	setExceptionHandlers16

	mov		dh, 0x7
	mov		si, msgPreStart
	call	println16
	;get drive params
	mov		ah, 0x48
	mov		si, bootDriveParams
	int		0x13
	jnc		.c1
	mov		dh, 0xc
	mov		si, msgBParamsF
	call	println16
	jmp		keyReboot16
	.c1:
	;get extensions available
	mov		dl, BYTE[bootDrive]
	mov		ah, 0x41
	mov		bx, 0x55aa
	int		0x13
	jnc		.c2
	mov		dh, 0xc
	mov		si, msgBNoExt
	call	println16
	jmp		keyReboot16
	.c2:

	call	clearKBBuffer16

	mov		dh, 0xf
	mov		si, msgMenu
	call	println16

	mov		dx, 20
	call	sleepKBInt16
	test	ax, 1
	jnz		.menu
	call	delLastLine16
	jmp		.option1_nomenu

	.menu:
	call	clearKBBuffer16

	mov		dh, 0
	call	clearScreen16

	call	printTitle16

	xor		bx, bx

	mov		bl, 3
	mov		dh, 0xa
	mov		si, msgBoot
	call	printAt16

	mov		bl, 4
	mov		dh, 0x7
	mov		si, msgSelPart
	call	printAt16

	mov		bl, 6
	mov		dh, 0xc
	mov		si, msgSelRB
	call	printAt16

	mov		cx, 5
	.wait:
	push	cx
	xor		bx, bx
	mov		bl, 23
	mov		dh, 0x7
	mov		si, msgAuto
	call	printAt16

	add		cx, 48
	mov		dl, cl
	mov		bh, (msgAutoEnd - msgAuto - 1)
	call	printCharAt16

	inc		bh
	mov		si, msgAutoEnd
	call	printAt16

	pop		cx
	mov		dx, 20
	call	sleepKBInt16
	test	ax, 1
	jnz		.key
	loop	.wait
	jmp		.option1

	.key:
	xor		ah, ah
	int		0x16
	cmp		al, 49
	je		.option1 ;startup
	cmp		al, 50
	je		.option2 ;select boot part
	cmp		al, 51
	je		.option3 ;reboot
	jmp		.wait

	.option1:
	call	printNln16
	.option1_nomenu:
	mov		dh, 0x7
	mov		si, msgBooting
	call	printp16

	mov		eax, DWORD[0x7db8]
	mov		DWORD[bootDriveSg], eax

	xor		edx, edx
	mov		dl, BYTE[bootPartN]
	call	getDec16_32
	mov		dh, 0x7
	call	print16
	call	printNln16

	cmp		BYTE[bootGPT], 1
	jae		.option1_gpt

	.option1_mbr:
	mov		ax, 0x10
	mul		BYTE[bootPartN]
	add		ax, 0x7dbe
	mov		bx, ax

	mov		edx, DWORD[bx + 8]
	mov		DWORD[bootPstart], edx

	mov		al, BYTE[bx + 4]
	mov		BYTE[bootPartFS], al

	;FAT12: 0x01 e.a.
	;FAT16: 0x04 0x06 0x0E e.a.
	;FAT32: 0x0B 0x0C e.a.
	;exFAT: 0x07 e.a.
	cmp		al, 0x04
	je		boot16
	cmp		al, 0x06
	je		boot16
	cmp		al, 0x0e
	je		boot16

	mov		dh, 0xc
	mov		si, msgUnsuppFS
	call	printp16
	mov		dl, al
	call	getFS16
	call	printlns16

	jmp		keyReboot16

	.option1_gpt:
	mov		edx, 1
	mov		ebx, 0x7e00
	mov		cx, 33
	call	readSectors16
	jc		.diskIOErr
	mov		al, BYTE[0x7e50]
	mov		BYTE[bootPartC], al

	mov		eax, DWORD[0x7e54]
	mul		BYTE[bootPartN]
	add		eax, 0x8000
	mov		ebx, eax
	mov		edx, DWORD[ebx + 0x20]
	mov		DWORD[bootPstart], edx

	cmp		edx, 1
	jbe		.option1_gpt_invalid

	mov		cx, 1
	mov		ebx, 0x7c00
	call	readSectors16
	jc		.diskIOErr

	mov		cx, 8
	mov		si, 0x7c36
	mov		di, fat16Sign
	.option1_gpt_verifyFat:
	lodsb
	cmp		BYTE[di], al
	jne		.option1_gpt_unsupported
	inc		di
	loop	.option1_gpt_verifyFat
	mov		BYTE[bootPartFS], 0x4

	jmp		boot16

	.option1_gpt_unsupported:
	mov		dh, 0xc
	mov		si, msgNotFat16
	call	println16

	jmp		keyReboot16

	.option1_gpt_invalid:
	mov		dh, 0xc
	mov		si, msgInvPart
	call	println16

	jmp		keyReboot16

	.option2:
	mov		dh, 0
	call	clearScreen16
	call	printTitle16
	test	BYTE[bootGPT], 1
	jz		.option2_mbr

	.option2_gpt:
	mov		edx, 1
	mov		ebx, 0x7e00
	mov		cx, 33
	call	readSectors16
	jc		.diskIOErr
	mov		al, BYTE[0x7e50]
	mov		BYTE[bootPartC], al
	mov		cx, 0
	.option2_gpt_printPartitions:
	push	cx
	mov		ax, WORD[0x7e54]
	mul		cx
	mov		si, 0x8038
	add		si, ax
	mov		di, gptPartName
	mov		cx, 36
	.option2_gpt_copyName:
	lodsb
	mov		BYTE[di], al
	inc		si
	inc		di
	loop	.option2_gpt_copyName
	pop		cx
	xor		edx, edx
	mov		dl, cl
	cmp		dl, 9
	jbe		.option2_printPartitions_c1
	add		dl, 39
	.option2_printPartitions_c1:
	add		dl, 48
	mov		dh, 0x7
	mov		bl, cl
	add		bl, 4
	mov		bh, 1
	call	printCharAt16
	mov		bh, 5
	mov		si, gptPartName
	call	printAt16
	cmp		cl, BYTE[bootPartN]
	jne		.option2_gpt_printPartitions_notStart
	mov		bh, 45
	mov		dh, 0xa
	mov		dl, '*'
	call	printCharAt16
	.option2_gpt_printPartitions_notStart:
	inc		cx
	cmp		cx, 20
	jae		.option2_gpt_printPartE
	cmp		cx, WORD[0x7e50]
	jb		.option2_gpt_printPartitions
	.option2_gpt_printPartE:
	mov		bl, 3
	mov		dh, 0x7
	call	printLine16

	mov		bl, 2
	mov		bh, 1
	mov		dh, 0x7
	mov		si, partSelID
	call	printAt16
	mov		bh, 5
	mov		si, partSelGNam
	call	printAt16
	mov		bh, 45
	mov		si, partSelSeld
	call	printAt16
	jmp		.option2_getKey

	.option2_mbr:
	mov		cx, 4
	.option2_printPartitions:
	call	main16_option2_printPartitions_getData

	;ID
	mov		ax, cx
	mov		dl, 2
	mul		dl
	add		ax, 2
	mov		bl, al
	mov		dh, 0x7
	call	printLine16
	inc		bl
	mov		dl, cl
	add		dl, 47
	xor		bh, bh
	call	printCharAt16

	;Size
	mov		bh, 5
	mov		edx, DWORD[mbrPartSizeKB]
	call	getDec16_32
	mov		dh, 0x7
	call	printAt16

	;file system
	mov		bh, 20
	mov		dl, BYTE[mbrPartFS]
	call	getFS16
	call	printAt16

	;boot flag
	cmp		BYTE[mbrPartBoot], 0x80
	jne		.option2_notBoot
	mov		bh, 50
	mov		dl, '*'
	call	printCharAt16
	.option2_notBoot:

	;current selected
	xor		ax, ax
	mov		al, cl
	dec		al
	cmp		al, BYTE[bootPartN]
	jne		.option2_notStart
	mov		bh, 60
	mov		dh, 0xa
	mov		dl, '*'
	call	printCharAt16
	.option2_notStart:
	loop	.option2_printPartitions

	mov		bl, 3
	xor		bh, bh
	mov		dh, 0x7
	mov		si, partSelID
	call	printAt16
	mov		bh, 5
	mov		si, partSelSize
	call	printAt16
	mov		bh, 20
	mov		si, partSelFS
	call	printAt16
	mov		bh, 50
	mov		si, partSelBoot
	call	printAt16
	mov		bh, 60
	mov		si, partSelSeld
	call	printAt16

	.option2_getKey:
	call	clearKBBuffer16
	xor		ah, ah
	int		0x16

	cmp		al, 0x1b
	je		.menu

	cmp		al, 97
	jb		.option2_getKey_num
	sub		al, 39

	.option2_getKey_num:
	cmp		al, 48
	jl		.option2_getKey
	mov		dl, BYTE[bootPartC]
	add		dl, 48
	cmp		al, dl
	jae		.option2_getKey
	sub		al, 48
	mov		BYTE[bootPartN], al

	;write sector at 0x7e00 to sector 2 (length 1)
	;mov		ax, 0x0301 ;function 3, 1 to write
	;mov		cx, 0x0002 ;cylinder 0, sector 2
	;mov		dh, 0
	;mov		dl, BYTE[bootDrive]
	;mov		bx, 0x7e00
	;int		0x13
	stc
	mov		ah, 1

	jnc		.menu
	mov		si, partSelPF
	mov		dh, 0xc
	call	printNln16
	call	println16
	mov		si, partSelPF2
	call	printp16
	xor		edx, edx
	mov		dl, ah
	call	getDec16_32
	mov		dh, 0x7
	call	printlns16
	mov		dx, 20
	call	sleep16
	jmp		.menu

	.option3:
	int		0x19
	cli
	hlt

	.diskIOErr:
	mov		dh, 0xc
	mov		si, bootDIOErr
	call	println16
	jmp		keyReboot16



keyReboot16:
	mov		dh, 0xf
	mov		si, msgAKReboot
	call	println16

	call	clearKBBuffer16
	xor		ah, ah
	int		0x16

	int		0x19
	cli
	hlt

origInvOpHandlerSeg	dw	0
origInvOpHandlerOff	dw	0

setExceptionHandlers16:
	push	dx
	mov		dx, WORD[0x1a]
	mov		WORD[origInvOpHandlerSeg], dx
	mov		dx, WORD[0x18]
	mov		WORD[origInvOpHandlerOff], dx
	mov		WORD[0x1a], 0
	mov		WORD[0x18], invalidOpcode16
	pop		dx
	ret

resetExceptionHandlers16:
	push	dx
	mov		dx, WORD[origInvOpHandlerSeg]
	mov		WORD[0x1a], dx
	mov		dx, WORD[origInvOpHandlerOff]
	mov		WORD[0x18], dx
	pop		dx
	ret

invalidOpcode16:
	mov		dh, 0x4
	mov		si, msgInvOp
	call	println16
	mov		si, msgExcept
	call	println16
	cli
	hlt

unknownException16: ; unused
	mov		dh, 0x4
	mov		si, msgUnExcept
	call	println16
	mov		si, msgExcept
	call	println16
	cli
	hlt

;------------------------------------------------------------
;     BOOT
;------------------------------------------------------------

msgStart	db	"Startup", 0

msgA20NoSt	db	"Failed to get A20 status", 0
msgA20Fail	db	"Failed to enable A20", 0

msgMemFail	db	"Failed to get memory size", 0

msgMapFail	db	"Failed to get memory map", 0

initialize_A20:
	mov		ax, 0x2401
	int		0x15
	jnc		.bios_success
	cli
	call	keybrd_in_wait
	mov		al, 0xAD
	out		0x64, al
	call	keybrd_in_wait
	mov		al, 0xD0
	out		0x64, al
	call	keybrd_out_wait
	in		al, 0x60
	push	eax
	call	keybrd_in_wait
	mov		al, 0xD1
	out		0x64, al
	call	keybrd_in_wait
	pop		eax
	or		al, 2
	out		0x60, al
	call	keybrd_in_wait
	mov		al, 0xAE
	out		0x64, al
	call	keybrd_in_wait
	sti
	.bios_success:
	mov		ax, 0x2402
	int		0x15
	jnc		.testA20
	mov		dh, 0xc
	mov		si, msgA20NoSt
	call	println16
	jmp		.end
	.testA20:
	test	al, 1
	jnz		.end
	mov		dh, 0xc
	mov		si, msgA20Fail
	call	println16
	stc
	.end:
	ret

keybrd_in_wait:
	in		al, 0x64
	test	al, 2
	jnz		keybrd_in_wait
	ret

keybrd_out_wait:
	in		al, 0x64
	test	al, 1
	jz		keybrd_out_wait
	ret

get_mem_size:
	pusha
	xor		ecx, ecx
	mov		ax, 0xe801
	int		0x15
	jc		.error
	jcxz	.mem_in_ab
	mov		WORD[mem_kb1], cx
	mov		WORD[mem_kb2], dx
	jmp		.end
	.mem_in_ab:
	mov		WORD[mem_kb1], ax
	mov		WORD[mem_kb2], bx
	.end:
	popa
	ret
	.error:
	popa
	mov		dh, 0xc
	mov		si, msgMemFail
	call	println16
	stc
	ret

get_mem_map:
	pushad
	push	es
	mov		DWORD[mmapStart], 0x70000
	mov		dx, 0x7000
	mov		es, dx
	mov		di, 0
	mov		WORD[mmapLength], 1
	xor		ebx, ebx
	mov		eax, 0xe820
	mov		ecx, 24
	mov		edx, 'PAMS'
	int		0x15
	add		di, 24
	jc		.error
	cmp		eax, 'PAMS'
	jne		.error
	cmp		ebx, 0
	je		.error
	.next:
	inc		WORD[mmapLength]
	mov		eax, 0xe820
	mov		ecx, 24
	mov		edx, 'PAMS'
	int		0x15
	.start:
	jcxz	.skip
	add		di, 24
	.skip:
	cmp		ebx, 0
	jne		.next
	jmp		.end
	.error:
	mov		dh, 0xc
	mov		si, msgMapFail
	call	println16
	stc
	.end:
	pop		es
	popad
	ret

boot16:
	mov		dh, 0x7
	mov		si, msgStart
	call	println16

	call	initialize_A20
	jc		.fail

	call	get_mem_size
	jc		.fail

	call	get_mem_map
	jc		.fail

	call	loadS2

	jmp		bootTo32
	.fail:
	cli
	hlt

;------------------------------------------------------------
;     FAT 16 BOOT
;------------------------------------------------------------

%define s2bootStart 0x30000
%define s2bootStartVirt 0x00030000
%define s2bootEndVirt 0x00060000
%define s2bootMaxSize s2bootEndVirt - s2bootStartVirt

bootFS1		db	"Detected file system: ", 0

bootLoad	db	"Loading /boot/s2boot ", 0
bootFail	db	"Failed to load /boot/s2boot", 0

bootLoadB	db	"Loading /boot/bdd.ko ", 0
bootFailB	db	"Failed to load /boot/bdd.ko", 0

bootTooLarg	db	"Boot files too large", 0

bootDIOErr	db	"Disk I/O error", 0
bootDIOErr2	db	"Error ", 0

bootPstart	dd	0

bootFAT16D1	db	"BOOT       "
bootFAT16F	db	"S2BOOT     "
bootFAT16FB	db	"BDD     KO "

FAT16data	dd	0

;------------------------------------------------------------
;     FAT 16
;------------------------------------------------------------

%define bytesPerSector 0x7c0b
%define sectorsPerCluster 0x7c0d
%define reservedSectors 0x7c0e
%define numberOfFATs 0x7c10
%define rootEntries 0x7c11
%define sectors1 0x7c13
%define media 0x7c15
%define sectorsPerFAT 0x7c16
%define sectorsPerTrack 0x7c18
%define headsPerCylinder 0x7c1a
%define hiddenSectors 0x7c1c
%define sectors2 0x7c20
%define driveNo 0x7c24

%define fat16TempData 0x30000

msgFAT16e1	db	"MBR.part.relative does not equal VBR_fat16.hidden, this might cause problems", 0
msgFAT16e2	db	"bootDriveParams.bytesPerSector does not equal VBR_fat16.bytesPerSector, this might cause problems", 0

fat16readDir	dw	0
fat16readFile	dw	0
fat16readDest	dd	0

fat16readFSize	dd	0

loadFAT16File_setDS:
	pushfd
	push	ebx
	shr		ebx, 4
	mov		ds, bx
	pop		ebx
	and		ebx, 0xf
	popfd
	ret

loadFAT16File_resetDS:
	pushfd
	push	edx
	xor		edx, edx
	mov		dx, ds
	shl		edx, 4
	add		dx, bx
	mov		ebx, edx
	mov		dx, 0
	mov		ds, dx
	pop		edx
	popfd
	ret

loadFAT16File:
	.root_init1:
	call	updateLoadingWheel16
	xor		eax, eax
	mov		ax, WORD[rootEntries]
	mov		dx, 32
	mul		dx
	xor		edx, edx
	div		WORD[bytesPerSector]
	mov		ecx, eax
	mov		al, BYTE[numberOfFATs]
	mul		WORD[sectorsPerFAT]
	add		ax, WORD[reservedSectors]
	add		eax, DWORD[bootPstart]
	mov		edx, eax
	mov		ebx, fat16TempData
	call	readSectors16
	jc		.read_fail

	mov		ebx, fat16TempData
	mov		cx, WORD[rootEntries]
	.root_read1:
	call	updateLoadingWheel16
	push	cx
	mov		cx, 11
	mov		si, WORD[fat16readDir]
	push	ebx
	call	loadFAT16File_setDS
	.root_read1_comp:
	mov		al, BYTE[es:si]
	cmp		al, BYTE[bx]
	jne		.root_read1_comp_fail
	inc		bx
	inc		si
	loop	.root_read1_comp
	clc
	jmp		.root_read1_comp_e
	.root_read1_comp_fail:
	stc
	.root_read1_comp_e:
	call	loadFAT16File_resetDS
	pop		ebx
	pop		cx
	jnc		.root_read1_s
	add		ebx, 0x20
	loop	.root_read1
	jmp		.fail
	.root_read1_s:
	push	ebx
	call	updateLoadingWheel16
	mov		ax, WORD[rootEntries]
	mov		dx, 32
	mul		dx
	xor		edx, edx
	div		WORD[bytesPerSector]
	mov		dx, ax
	push	dx
	xor		ax, ax
	mov		al, BYTE[numberOfFATs]
	mul		WORD[sectorsPerFAT]
	pop		dx
	add		ax, dx
	add		ax, WORD[reservedSectors]
	add		eax, DWORD[bootPstart]
	mov		DWORD[FAT16data], eax
	pop		ebx
	.root_init2:
	call	updateLoadingWheel16
	call	loadFAT16File_setDS
	mov		ax, WORD[bx + 0x1a]
	call	loadFAT16File_resetDS
	sub		ax, 2
	mul		BYTE[sectorsPerCluster]
	add		eax, DWORD[FAT16data]
	mov		edx, eax
	xor		ecx, ecx
	mov		cl, BYTE[sectorsPerCluster]
	mov		ebx, fat16TempData
	call	readSectors16
	jc		.read_fail

	xor		eax, eax
	xor		edx, edx
	mov		al, BYTE[sectorsPerCluster]
	mul		WORD[bytesPerSector]
	mov		cx, 32
	div		cx
	mov		cx, ax
	mov		ebx, fat16TempData
	.root_read2:
	call	updateLoadingWheel16
	push	cx
	mov		cx, 11
	mov		si, WORD[fat16readFile]
	push	ebx
	call	loadFAT16File_setDS
	.root_read2_comp:
	mov		al, BYTE[es:si]
	cmp		al, BYTE[bx]
	jne		.root_read2_comp_fail
	inc		bx
	inc		si
	loop	.root_read2_comp
	clc
	jmp		.root_read2_comp_e
	.root_read2_comp_fail:
	stc
	.root_read2_comp_e:
	call	loadFAT16File_resetDS
	pop		ebx
	pop		cx
	jnc		.fat_init
	add		ebx, 0x20
	loop	.root_read2
	jmp		.fail
	.fat_init:
	call	updateLoadingWheel16
	call	loadFAT16File_setDS
	mov		edx, DWORD[bx + 0x1c]
	call	loadFAT16File_resetDS
	mov		DWORD[fat16readFSize], edx
	xor		edx, edx
	call	loadFAT16File_setDS
	mov		dx, WORD[bx + 0x1a]
	call	loadFAT16File_resetDS
	cmp		dx, 2
	jb		.fail
	push	dx
	xor		edx, edx
	add		edx, DWORD[bootPstart]
	add		dx, WORD[reservedSectors]
	xor		ecx, ecx
	mov		cx, WORD[sectorsPerFAT]
	mov		ebx, fat16TempData
	call	readSectors16
	jc		.read_fail

	mov		bx, 0x7e00
	pop		dx
	mov		WORD[bx], dx
	add		bx, 2
	.fat_read:
	call	updateLoadingWheel16
	xor		eax, eax
	mov		ax, WORD[bx - 2]
	add		eax, eax
	add		eax, fat16TempData
	push	bx
	mov		ebx, eax
	call	loadFAT16File_setDS
	mov		dx, WORD[bx]
	call	loadFAT16File_resetDS
	pop		bx
	mov		WORD[bx], dx
	cmp		dx, 0xfff8
	jae		.data_init
	add		bx, 2
	cmp		bx, 0x8000
	jae		.fail
	jmp		.fat_read
	.data_init:
	call	updateLoadingWheel16
	mov		si, 0x7e00
	xor		ebx, ebx
	mov		ebx, DWORD[fat16readDest]
	.data_read:
	call	updateLoadingWheel16
	xor		eax, eax
	mov		ax, WORD[si]
	sub		ax, 2
	xor		dx, dx
	mov		dl, BYTE[sectorsPerCluster]
	mul		dx
	add		eax, DWORD[FAT16data]
	mov		edx, eax
	xor		ecx, ecx
	mov		cl, BYTE[sectorsPerCluster]
	push	ebx
	call	readSectors16
	pop		ebx
	jc		.read_fail
	cmp		WORD[si + 2], 0xfff8
	jae		.success
	add		si, 2
	xor		eax, eax
	mov		al, BYTE[sectorsPerCluster]
	mul		WORD[bytesPerSector]
	add		ebx, eax
	jmp		.data_read
	.fail:
	call	printNln16
	stc
	ret
	.read_fail:
	push	ax
	call	printNln16
	jmp		read_fail
	.success:
	call	printNln16
	clc
	ret

loadS2:
	; print boot partition file system
	mov		al, BYTE[bootPartFS]
	mov		dh, 0x7
	mov		si, bootFS1
	call	printp16
	mov		dl, al
	call	getFS16
	call	printlns16

	; read vbr of boot partition
	mov		edx, DWORD[bootPstart]
	mov		cx, 1
	mov		ebx, 0x7c00
	call	readSectors16
	jc		.read_fail

	; check if partition offset in partition table equals number of hidden sectors in FAT header (print warning if not)
	mov		edx, DWORD[hiddenSectors]
	cmp		edx, DWORD[bootPstart]
	je		.c1
	mov		dh, 0xc
	mov		si, msgFAT16e1
	call	println16

	; check if number of bytes per sectors given with int 13h equals bytes per sector in FAT header (print warning if not)
	.c1:
	mov		dx, WORD[bytesPerSector]
	cmp		dx, WORD[bBytesPSec]
	je		.c2
	mov		dh, 0xc
	mov		si, msgFAT16e2
	call	println16

	.c2:
	mov		dh, 0x7
	mov		si, bootLoad
	call	printp16

	; load s2boot to s2bootFileLoad
	mov		DWORD[fat16readDest], s2bootFileLoad
	mov		WORD[fat16readDir], bootFAT16D1
	mov		WORD[fat16readFile], bootFAT16F
	call	loadFAT16File
	jc		.s2_load_fail

	; check if s2boot (raw) end address overwrites the location where s2boot will be loaded
	mov		edx, DWORD[fat16readFSize] ; set to file size of last file read by loadFAT16File
	mov		DWORD[s2bSize], edx
	and		edx, 0xffffff00
	add		edx, 0x8100
	cmp		edx, s2bootStart
	jae		.files_too_large

	push	edx
	mov		dh, 0x7
	mov		si, bootLoadB
	call	printp16
	pop		edx

	; load bdd after end of s2boot (0x100 aligned)
	mov		DWORD[bddAddress], edx
	mov		DWORD[fat16readDest], edx
	mov		WORD[fat16readDir], bootFAT16D1
	mov		WORD[fat16readFile], bootFAT16FB
	call	loadFAT16File
	jc		.bdd_load_fail

	mov		edx, DWORD[fat16readFSize]
	mov		DWORD[bddSize], edx

	; check if s2boot (raw) + bdd end address overwrites the location where s2boot will be loaded
	mov		edx, DWORD[fat16readDest]
	add		edx, DWORD[fat16readFSize]
	cmp		edx, s2bootStart
	jae		.files_too_large
	ret
	.s2_load_fail:
	mov		si, bootFail
	jmp		load_fail
	.bdd_load_fail:
	mov		si, bootFailB
	jmp		load_fail
	.files_too_large:
	mov		si, bootTooLarg
	jmp		load_fail
	.read_fail:
	push	ax
	jmp		read_fail

load_fail:
	mov		dh, 0xc
	call	println16
	cli
	hlt

read_fail:
	mov		dh, 0xc
	mov		si, bootDIOErr
	call	println16
	mov		si, bootDIOErr2
	call	printp16
	xor		edx, edx
	pop		ax
	mov		dl, ah
	call	getDec16_32
	mov		dh, 0xc
	call	printlns16
	cli
	hlt




;------------------------------------------------------------
;     BOOT TO 32
;------------------------------------------------------------

bits 16

gdt32_start:
	;null
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	
	;code
	db 11111111b
	db 11111111b
	db 00000000b
	db 00000000b
	db 00000000b
	db 10011010b
	db 11001111b
	db 00000000b
	
	;data
	db 11111111b
	db 11111111b
	db 00000000b
	db 00000000b
	db 00000000b
	db 10010010b
	db 11001111b
	db 00000000b
gdt32_end:

gdt32:
	dw gdt32_end - gdt32_start - 1
	dq gdt32_start

idt32_start:
	; div by 0
	dw unknownException32
	dd 0x0e000008
	dw 0

	; ?
	dw unknownException32
	dd 0x0e000008
	dw 0
	
	; nmi
	dw unknownException32
	dd 0x0e000008
	dw 0
	
	; breakpoint
	dw unknownException32
	dd 0x0e000008
	dw 0

	; overflow
	dw unknownException32
	dd 0x0e000008
	dw 0

	; bounds range exception
	dw unknownException32
	dd 0x0e000008
	dw 0

	; inv opcode
	dw invalidOpcode32
	dd 0x8e000008
	dw 0

	; device not available
	dw unknownException32
	dd 0x0e000008
	dw 0

	; double fault
	dw doubleFault32
	dd 0x8e000008
	dw 0

	; coprocessor seg overrun
	dw unknownException32
	dd 0x0e000008
	dw 0

	; invalid tss
	dw unknownException32
	dd 0x0e000008
	dw 0

	; segment not present
	dw unknownException32
	dd 0x0e000008
	dw 0

	; stack-segment fault
	dw unknownException32
	dd 0x0e000008
	dw 0

	; general protection fault
	dw generalPF32
	dd 0x8e000008
	dw 0

	; page fault
	dw pageFault32
	dd 0x8e000008
	dw 0

	; ?2
	dw unknownException32
	dd 0x0e000008
	dw 0

	; x87 fpu error
	dw unknownException32
	dd 0x0e000008
	dw 0

	; alignment check
	dw unknownException32
	dd 0x0e000008
	dw 0

	; machine check
	dw unknownException32
	dd 0x0e000008
	dw 0

	; simd fp exception
	dw unknownException32
	dd 0x0e000008
	dw 0

	times 256 - ($ - idt32_start) db 0
idt32_end:

idt32:
	dw idt32_end - idt32_start - 1
	dd idt32_start


bootTo32:

	call	resetExceptionHandlers16

	cli
	lgdt	[gdt32]
	mov		eax, cr0
	or		eax, 1
	mov		cr0, eax

	jmp		0x8:boot32




;------------------------------------------------------------
;     BOOT 32
;------------------------------------------------------------

bits 32

msgELFS2		db	"Parsing ELF ", 0
msgInvELF		db	"Invalid ELF format in s2boot image", 0
msgInvBitS2		db	"s2boot image was compiled for 64bit, expected 32bit", 0
msgInvTpS2		db	"Expected type 2 (executable) or type 3 (dynamic) for s2boot image", 0
msgInvISS2		db	"s2boot image was not compiled for x86", 0
msgNumSkipSeg1	db	"Skipped ", 0
msgNumSkipSeg2	db	" invalid segments", 0
msgNumSeg1		db	"Loaded ", 0
msgNumSeg2		db	" segments", 0
msgNoSeg		db	"Loading failed", 0
msgS2			db	"Calling s2boot", 0
msgS2Return		db	"s2boot returned: ", 0

s2bootDynSeg	dd	0
s2bootDynSegLen	dd	0
s2bootRelaBase	dd	0
s2bootRelaLen	dd	0
skipSegments	db	0
numSegments		db	0
s2bootEntry		dd	0

dec_out			db	"0000000000000000000000", 0
hex_out			db	"0x00000000 ", 0

msgErrCP		db	"err=", 0
msgErrCP2		db	"eip=", 0
errCode			dd	0
errEIP			dd	0


print32: ;pointer in esi, attributes in dh
	pusha
	.l:
	lodsb
	cmp		al, 0
	je		.c
	mov		dl, al
	call	printChar32
	jmp		.l
	.c:
	jmp		complete32

printp32:
	pusha
	push	esi
	mov		esi, msgPre
	call	print32
	pop		esi
	call	print32
	jmp		complete32

println32:
	pusha
	push	esi
	mov		esi, msgPre
	call	print32
	pop		esi
	call	print32
	call	printNln32
	jmp		complete32

printChar32: ;data in dx dl-char dh-attr
	pusha
	
	mov		edi, 0xb8000
	xor		eax, eax
	xor		ecx, ecx
	mov		al, byte[cu_x]
	mov		cl, 2
	mul		cl
	add		edi, eax
	mov		al, byte[cu_y]
	mov		cl, 160
	mul		cl
	add		edi, eax

	cmp		dl, 0xa
	je		.c

	mov		word[edi], dx
	inc		byte[cu_x]
	cmp		byte[cu_x], 80
	je		.c
	jmp		complete32
	.c:
	call	printNln32
	jmp		complete32

printNln32:
	pusha
	mov		byte[cu_x], 0
	inc		byte[cu_y]
	cmp		byte[cu_y], 25
	je		.c
	jmp		complete32
	.c:
	call	shiftUp32
	jmp		complete32

shiftUp32:
	pusha
	dec		byte[cu_y]
	mov		edi, 0xb8000
	mov		cx, 960
	.l:
	mov		edx, dword[edi+160]
	mov		dword[edi], edx
	add		edi, 4
	loop	.l
	mov		cx, 40
	.l2:
	mov		dword[edi], 0x07200720
	add		edi, 4
	loop	.l2
	jmp		complete32

updateCursor32:
	pusha
	xor		eax, eax
	mov		al, byte[cu_y]
	mov		cl, 80
	mul		cl
	add		al, byte[cu_x]
	mov		ebx, eax

	mov		dx, 0x3d4
	mov		al, 0xe
	out		dx, al
	mov		dx, 0x3d5
	mov		al, bh
	out		dx, al

	mov		dx, 0x3d4
	mov		al, 0xf
	out		dx, al
	mov		dx, 0x3d5
	mov		al, bl
	out		dx, al

	popa
	ret

updateCursorMemData32:
	pusha
	mov		dx, 0x3d4
	mov		al, 0xe
	out		dx, al
	inc		dx
	in		al, dx
	mov		bh, al
	mov		dx, 0x3d4
	mov		al, 0xf
	out		dx, al
	inc		dx
	in		al, dx
	mov		bl, al
	mov		ax, bx
	mov		cx, 80
	xor		dx, dx
	div		cx
	mov		BYTE[cu_x], dl
	mov		BYTE[cu_y], al
	popa
	ret

updateLoadingWheel32:
	pusha

	mov		edi, 0xb8000
	xor		eax, eax
	xor		ecx, ecx
	mov		al, BYTE[cu_x]
	mov		cl, 2
	mul		cl
	add		edi, eax
	mov		al, BYTE[cu_y]
	mov		cl, 160
	mul		cl
	add		edi, eax

	mov		dx, WORD[edi]
	cmp		dl, '-'
	je		.minus
	cmp		dl, '/'
	je		.slash
	cmp		dl, 0x5c
	je		.back
	cmp		dl, '|'
	je		.line
	mov		dl, '-'
	mov		WORD[edi], dx
	jmp		complete32
	.minus:
	mov		dl, 0x5c
	jmp		.c
	.slash:
	mov		dl, '-'
	jmp		.c
	.back:
	mov		dl, '|'
	jmp		.c
	.line:
	mov		dl, '/'
	jmp		.c
	.c:
	mov		WORD[edi], dx
	jmp		complete32

complete32:
	call	updateCursor32
	popa
	ret

printDec32: ; number in edx
	pusha
	mov		edi, dec_out + 21
	.reset:
	mov		BYTE[edi], 0x30
	dec		edi
	cmp		edi, dec_out - 1
	jne		.reset
	mov		edi, dec_out + 21
	mov		ebx, 10
	mov		eax, edx
	.loop:
	xor		edx, edx
	div		ebx
	add		dl, 48
	mov		BYTE[edi], dl
	cmp		eax, 0
	je		.print
	dec		edi
	jmp		.loop
	.print:
	mov		esi, dec_out - 1
	.cut:
	inc		esi
	cmp		BYTE[esi], 0x30
	je		.cut
	cmp		esi, dec_out + 22
	jne		.p
	mov		esi, dec_out + 21
	.p:
	mov		dh, 00000101b
	call	print32
	popa
	ret

printHex32:
	push	eax
	push	ebx
	push	ecx
	push	edx
	push	esi

	mov		ecx, 8
	.char_loop:
	mov		eax, edx
	shr		edx, 4
	and		eax, 0xf

	mov		ebx, hex_out
	add		ebx, 1
	add		ebx, ecx

	mov		BYTE[ebx], 0x30

	cmp		ax, 0xa
	jl		.set_letter
	add		byte[ebx], 7
	jl		.set_letter
	.set_letter:
	add		byte[ebx],al 
	loop	.char_loop
	jmp		.print_hex_done
	.print_hex_done:
	mov		ecx, 8
	mov		esi, hex_out
	add		esi, 2
	.cmp:
	cmp		BYTE[esi], 0x30
	jne		.p
	inc		esi
	loop	.cmp
	;jmp		.ret
	.p:
	mov		esi, hex_out
	mov		dh, 00001101b
	call	print32
	.ret:
	pop		esi
	pop		edx
	pop		ecx
	pop		ebx
	pop		eax
	ret

invalidOpcode32:
	pop		DWORD[errEIP]
	mov		esi, msgInvOp
	jmp		exception32

doubleFault32:
	pop		DWORD[errCode]
	pop		DWORD[errEIP]
	mov		esi, msgDoubleF
	jmp		exception32

generalPF32:
	pop		DWORD[errCode]
	pop		DWORD[errEIP]
	mov		esi, msgGPF
	jmp		exception32

pageFault32:
	pop		DWORD[errCode]
	pop		DWORD[errEIP]
	mov		esi, msgPF
	jmp		exception32

unknownException32: ; this is unused
	pop		DWORD[errCode]
	pop		DWORD[errEIP]
	mov		esi, msgUnExcept
	jmp		exception32

exception32:
	call	updateCursorMemData32
	mov		dh, 0x4
	call	println32
	call	printErrC32
	mov		dh, 0x4
	mov		esi, msgExcept
	call	println32
	cli
	hlt

printErrC32:
	mov		dh, 0xf
	mov		esi, msgErrCP
	call	printp32
	mov		edx, DWORD[errCode]
	call	printHex32
	mov		dh, 0xf
	mov		esi, msgErrCP2
	call	print32
	mov		edx, DWORD[errEIP]
	call	printHex32
	call	printNln32
	ret

parseELF32:
	call	updateLoadingWheel32
	cmp		DWORD[s2bootFileLoad], 0x464c457f
	jne		.invalid_format
	cmp		BYTE[s2bootFileLoad + 0x04], 0x01
	jne		.invalid_bit
	cmp		WORD[s2bootFileLoad + 0x05], 0x0101
	jne		.invalid_format
	cmp		WORD[s2bootFileLoad + 0x10], 0x0002
	je		.valid_type
	cmp		WORD[s2bootFileLoad + 0x10], 0x0003
	je		.valid_type
	jmp		.invalid_type
	.valid_type:
	cmp		WORD[s2bootFileLoad + 0x12], 0x0003
	jne		.invalid_instruction_set
	call	updateLoadingWheel32
	mov		DWORD[s2mapStart], s2bootMapLocation
	mov		DWORD[s2bootBase], s2bootStartVirt
	movzx	ecx, WORD[s2bootFileLoad + 0x2c] ; number of ph table entries
	mov		ebx, s2bootFileLoad
	add		ebx, DWORD[s2bootFileLoad + 0x1c] ; start of ph table
	.parse_program_header:
	call	updateLoadingWheel32
	push	ebx
	push	ecx
	cmp		DWORD[ebx], 1 ; loadable segment
	jne		.check_dyn
	mov		eax, DWORD[ebx + 0x08] ; target address
	add		eax, DWORD[ebx + 0x14] ; size in memory
	cmp		eax, s2bootMaxSize ; check if target address + size is below maximum size
	jb		.segm_c_2
	inc		BYTE[skipSegments]
	jmp		.parse_program_header_f
	.segm_c_2:
	; add s2map entry
	mov		eax, 8
	mul		WORD[s2mapLength]
	add		eax, DWORD[s2mapStart]
	mov		edi, eax
	mov		edx, DWORD[ebx + 0x08] ; target address
	add		edx, s2bootStartVirt
	mov		DWORD[edi], edx
	add		edx, DWORD[ebx + 0x14] ; size in memory
	mov		DWORD[edi + 4], edx
	inc		WORD[s2mapLength]

	mov		ecx, DWORD[ebx + 0x10] ; size in file
	mov		edi, DWORD[ebx + 0x08] ; target address
	add		edi, s2bootStartVirt
	mov		esi, DWORD[ebx + 0x04] ; offset in file
	add		esi, s2bootFileLoad
	cld
	.copy_segm:
	call	updateLoadingWheel32
	movsb
	loop	.copy_segm
	inc		BYTE[numSegments]
	jmp		.parse_program_header_f
	.check_dyn:
	cmp		DWORD[ebx], 2 ; dynamic linker segment
	jne		.parse_program_header_f
	mov		edx, DWORD[ebx + 0x04] ; offset in file
	mov		DWORD[s2bootDynSeg], edx
	mov		edx, DWORD[ebx + 0x10] ; size in file
	mov		DWORD[s2bootDynSegLen], edx
	.parse_program_header_f:
	pop		ecx
	pop		ebx
	movzx	edx, WORD[s2bootFileLoad + 0x2a] ; size of ph entry
	add		ebx, edx
	dec		ecx
	jnz		.parse_program_header

	call	updateLoadingWheel32
	mov		edx, DWORD[s2bootFileLoad + 0x18] ; entry point address
	add		edx, s2bootStartVirt
	mov		DWORD[s2bootEntry], edx

	cmp		DWORD[s2bootDynSeg], 0
	je		.link_f
	; dynamic link
	call	updateLoadingWheel32
	mov		eax, DWORD[s2bootDynSegLen]
	xor		edx, edx
	mov		edi, 8
	div		edi ; length of entry
	mov		ecx, eax
	mov		ebx, DWORD[s2bootDynSeg]
	add		ebx, s2bootFileLoad
	.parseDynSec:
	call	updateLoadingWheel32
	cmp		DWORD[ebx], 17 ; rel section
	je		.parseDynSec_relaBase
	cmp		DWORD[ebx], 18 ; rel section length
	je		.parseDynSec_relaLen
	jmp		.parseDynSec_f
	.parseDynSec_relaBase:
	mov		edx, DWORD[ebx + 4]
	mov		DWORD[s2bootRelaBase], edx
	jmp		.parseDynSec_f
	.parseDynSec_relaLen:
	mov		edx, DWORD[ebx + 4]
	mov		DWORD[s2bootRelaLen], edx
	jmp		.parseDynSec_f
	.parseDynSec_f:
	add		ebx, 8
	loop	.parseDynSec

	cmp		DWORD[s2bootRelaBase], 0
	je		.link_f
	call	updateLoadingWheel32
	mov		eax, DWORD[s2bootRelaLen]
	xor		edx, edx
	mov		edi, 8
	div		edi ; length of entry in rela section
	mov		ecx, eax
	mov		ebx, DWORD[s2bootRelaBase]
	add		ebx, s2bootStartVirt
	.setRela:
	xor		edx, edx
	mov		edx, DWORD[ebx + 4]
	cmp		edx, 8 ; type DYNL_R_386_RELATIVE (base + addend)
	jne		.setRela_f
	mov		edi, DWORD[ebx] ; addend address
	add		edi, s2bootStartVirt
	mov		edx, DWORD[edi] ; addend
	add		edx, s2bootStartVirt
	mov		DWORD[edi], edx
	.setRela_f:
	add		ebx, 8
	loop	.setRela

	.link_f:

	call	printNln32
	cmp		BYTE[skipSegments], 0
	jbe		.print_loaded_segs
	mov		dh, 0xe
	mov		esi, msgNumSkipSeg1
	call	printp32
	movzx	edx, BYTE[skipSegments]
	call	printDec32
	mov		dh, 0xe
	mov		esi, msgNumSkipSeg2
	call	print32
	call	printNln32
	.print_loaded_segs:
	mov		dh, 0x7
	mov		esi, msgNumSeg1
	call	printp32
	movzx	edx, BYTE[numSegments]
	call	printDec32
	mov		dh, 0x7
	mov		esi, msgNumSeg2
	call	print32
	call	printNln32
	cmp		BYTE[numSegments], 1
	jb		.no_segments ; 0 loaded segments, something definitely went wrong
	clc
	ret
	.invalid_format:
	mov		esi, msgInvELF
	jmp		.parse_error
	.invalid_bit:
	mov		esi, msgInvBitS2
	jmp		.parse_error
	.invalid_type:
	mov		esi, msgInvTpS2
	jmp		.parse_error
	.invalid_instruction_set:
	mov		esi, msgInvISS2
	jmp		.parse_error
	.no_segments:
	mov		esi, msgNoSeg
	jmp		.parse_error
	.parse_error:
	call	printNln32
	mov		dh, 0xc
	call	println32
	stc
	ret

; map (phys) 0x0000000000000000 - 0x0000000000400000 to (virt) 0x0000000000000000
; 0x00060000 - 0x00061000: page directory table
; 0x00061000 - 0x00062000: page table for first mapping
setupPaging:
	push	eax
	push	ecx
	push	edx
	push	edi
	; page directory table
	mov		DWORD[0x60000], 0x61003 ; phys address 0x61000, set present (0x1) and r/w (0x2) bit

	; page tables
	mov		ecx, 1024
	mov		edi, 0x61000
	mov		edx, 0x3
	.writePT:
	mov		DWORD[edi], edx
	add		edi, 4
	add		edx, 0x1000
	loop	.writePT

	mov		eax, 0x60000
	mov		cr3, eax

	mov		eax, cr0
	or		eax, 0x80000001
	mov		cr0, eax
	pop		edi
	pop		edx
	pop		ecx
	pop		eax
	ret



boot32:
	mov		ax, 0x10
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax
	mov		esp, 0x7c00

	lidt	[idt32]

	call	setupPaging

	mov		dh, 0x7
	mov		esi, msgELFS2
	call	printp32
	call	parseELF32
	jnc		.c1
	cli
	hlt

	.c1:

	mov		DWORD[membase], 0

	mov		dh, 0x7
	mov		esi, msgS2
	call	println32

	push	header_start
	mov		ebp, 0
	call	DWORD[s2bootEntry]

	.halt:
	cli
	hlt
	jmp		.halt




; SERVICE CALLBACK

servMemBuf	dd	0

reqBufStruct	dd	0
reqWidth	dd	0
reqHeight	dd	0
reqBPP		dd	0
reqMode		dd	0
reqResFramebuffer	dd	0
reqResBytesPerLine	dd	0
curVidMode	dw	0
gdt32temp:
	dw 0
	dd 0
idt32temp:
	dw 0
	dd 0
stack32temp	dd	0

retCode		dd	0

gdt16_start:
	;null
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	db 00000000b
	
	;code
	db 11111111b
	db 11111111b
	db 00000000b
	db 00000000b
	db 00000000b
	db 10011010b
	db 00001111b
	db 00000000b
	
	;data
	db 11111111b
	db 11111111b
	db 00000000b
	db 00000000b
	db 00000000b
	db 10010010b
	db 00001111b
	db 00000000b
gdt16_end:

gdt16:
	dw gdt16_end - gdt16_start - 1
	dq gdt16_start

idt16:
	dw 0x3ff
	dd 0

; services: (<di> - <description> (<up to 3 passed arguments>))
;  1 - upstream pre init (address to kmalloc)
;  2 - upstream init (<no args>)
;  10 - set video to closest (modeInfoStruct)
serviceCallback: ; di, si, dx, cx
	push	ebp
	mov		ebp, esp
	push	edi
	push	esi
	mov		edi, DWORD[ebp + 8]
	mov		esi, DWORD[ebp + 12]
	mov		edx, DWORD[ebp + 16]
	mov		ecx, DWORD[ebp + 20]
	mov		eax, 0
	cmp		edi, 1
	je		.uPreInit
	cmp		edi, 2
	je		.uInit
	cmp		edi, 10
	je		.setVideo
	mov		eax, 1
	jmp		.end

	.uPreInit:
	push	0x1000
	call	esi
	add		esp, 4
	mov		DWORD[servMemBuf], eax
	cmp		DWORD[servMemBuf], 0
	je		.uPreInit_err
	cmp		DWORD[servMemBuf], 0xffff
	ja		.uPreInit_err
	mov		eax, 0
	jmp		.end
	.uPreInit_err:
	mov		eax, 27 ; TSX_MEMORY_RESERVED
	jmp		.end

	.uInit:
	jmp		.end

	.setVideo:
	mov		DWORD[reqBufStruct], esi
	mov		edx, DWORD[esi]
	mov		DWORD[reqWidth], edx
	mov		edx, DWORD[esi + 4]
	mov		DWORD[reqHeight], edx
	mov		edx, DWORD[esi + 8]
	mov		DWORD[reqBPP], edx
	mov		edx, DWORD[esi + 12]
	mov		DWORD[reqMode], edx


	; go back to 16 bit to use vbe
	cli
	sgdt	[gdt32temp]
	sidt	[idt32temp]
	mov		DWORD[stack32temp], esp
	mov		esp, 0x7b00 ; set this in case the stack got relocated somewhere above 0xffff

	lgdt	[gdt16]
	jmp		0x8:.16bitprot
	bits	16
	.16bitprot:
	mov		eax, 0x10
	mov		ds, eax
	mov		es, eax
	mov		fs, eax
	mov		gs, eax
	mov		ss, eax

	mov		eax, cr0
	and		eax, 0x7ffffffe
	mov		cr0, eax

	jmp		0x0:.16bitreal
	.16bitreal:
	mov		ax, 0
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax
	lidt	[idt16]


	cmp		BYTE[reqMode], 1
	je		.nonStandardMode
	cmp		WORD[reqWidth], 80
	jne		.nonStandardMode
	cmp		WORD[reqHeight], 25
	jne		.nonStandardMode
	cmp		WORD[reqBPP], 16
	jne		.nonStandardMode
	; mode 3, text 80x25
	mov		DWORD[reqResFramebuffer], 0xb8000
	mov		WORD[reqResBytesPerLine], 160
	mov		ax, 0x0003 ; ah = 0 set video mode, al = 3 video mode number
	int		0x10
	jc		.vbe_err
	jmp		.vbe_done

	.nonStandardMode:
	mov		ax, 0x4f00
	mov		di, WORD[servMemBuf]
	int		0x10
	cmp		ax, 0x4f ; success
	jne		.vbe_err
	mov		bx, WORD[servMemBuf]
	add		bx, 0xe ; pointer to modes
	mov		es, WORD[bx + 2] ; segment
	mov		bx, WORD[bx]
	.checkModes:
	mov		dx, WORD[es:bx] ; dx = video mode num
	mov		WORD[curVidMode], dx
	cmp		dx, 0xffff
	je		.modeCheckDone
	push	bx
	push	es
	mov		cx, dx
	mov		ax, 0
	mov		es, ax
	mov		ax, 0x4f01
	mov		di, WORD[servMemBuf]
	add		di, 512 ; place this structure behind the controller information (which is 512 bytes)
	push	di
	int		0x10
	pop		di
	cmp		ax, 0x4f ; success
	jne		.checkModes_f
	; --- check ---
	; attributes
	mov		ax, 0x81 ; supported, linear frame buffer
	cmp		BYTE[reqMode], 1 ; 1 = graphics mode
	jne		.notGraphics
	or		ax, 0x10 ; graphics mode
	.notGraphics:
	mov		dx, WORD[di]
	and		dx, ax
	cmp		dx, ax

	jne		.checkModes_f
	; x res
	mov		dx, WORD[reqWidth]
	cmp		dx, WORD[di + 0x12]
	jne		.checkModes_f
	; y res
	mov		dx, WORD[reqHeight]
	cmp		dx, WORD[di + 0x14]
	jne		.checkModes_f
	; bpp
	mov		dl, BYTE[reqBPP]
	cmp		dl, BYTE[di + 0x19]
	jne		.checkModes_f
	; number of banks (must be 1 because framebuffer must be continuous block in memory)
	cmp		BYTE[di + 0x1a], 1
	jne		.checkModes_f
	; memory model (4 or 6)
	cmp		BYTE[di + 0x1b], 4
	je		.checkMemModel_f
	cmp		BYTE[di + 0x1b], 6
	je		.checkMemModel_f
	jmp		.checkModes_f
	.checkMemModel_f:
	; framebuffer address
	mov		edx, DWORD[di + 0x28]
	cmp		edx, 0
	je		.checkModes_f
	mov		DWORD[reqResFramebuffer], edx
	; bytes per scan line
	mov		dx, WORD[di + 0x10]
	mov		WORD[reqResBytesPerLine], dx
	pop		es
	pop		bx
	jmp		.modeCheckDone
	.checkModes_f:
	pop		es
	pop		bx
	add		bx, 2
	jmp		.checkModes

	.modeCheckDone:
	mov		edx, DWORD[reqResFramebuffer]
	cmp		edx, 0
	je		.vbe_err ; mode not found

	; set mode
	mov		ax, 0
	mov		es, ax
	mov		ax, 0x4f02
	mov		di, WORD[servMemBuf]
	add		di, 512 ; place this structure behind the controller information (which is 512 bytes)
	mov		bx, WORD[curVidMode]
	and		bx, 0x1ff
	or		bx, 0x4000 ; enable linear frame buffer
	int		0x10
	cmp		ax, 0x4f ; success
	jne		.vbe_err

	mov		WORD[retCode], 0
	jmp		.vbe_done
	.vbe_err:
	mov		WORD[retCode], 1
	.vbe_done:


	lgdt	[gdt32temp]
	mov		eax, cr0
	or		eax, 1
	mov		cr0, eax

	jmp		0x8:.32bitprot
	bits	32
	.32bitprot:
	mov		ax, 0x10
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax

	mov		esp, DWORD[stack32temp]
	lidt	[idt32temp]

	mov		esi, DWORD[reqBufStruct]
	mov		edx, DWORD[reqResFramebuffer]
	mov		DWORD[esi + 16], edx ; write framebuffer address to struct
	mov		edx, DWORD[reqResBytesPerLine]
	mov		DWORD[esi + 20], edx ; write bytes per line to struct

	mov		eax, DWORD[retCode]
	jmp		.end

	.end:
	pop		esi
	pop		edi
	pop		ebp
	ret










align 32, db 0xcc

s1bootEndAddr:
