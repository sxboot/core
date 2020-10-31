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

bits 32
org 0x5000

; VERSION 3

jmp main32

msgPre		db	"[s3] ", 0
msgMFail	db	"Unexpected error", 0
msgBoot		db	"Booting ", 0
msgUType	db	"Unknown type", 0
msgBye		db	"Thank you and goodbye", 0

s3dataLoc	dq	0

s3BMode		dw	0
s3jmpDest	dq	0
s3vaddrOff	dd	0
s3archFlags	dd	0
; entry state
s3entry_a	dd	0
s3entry_b	dd	0
s3entry_c	dd	0
s3entry_d	dd	0
s3entry_sp	dd	0
s3entry_bp	dd	0
s3entry_si	dd	0
s3entry_di	dd	0
s3entry_gdtb	dd	0
s3entry_gdtl	dd	0
s3entry_cs	dw	0
s3entry_ds	dw	0

cu_x	db	0
cu_y	db	0

idt:
	dw 0
	dq 0

align 32, db 0xcc



bits 32

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

clearScreen32: ;attributes in dh
	pusha
	mov		edi, 0xb8000
	mov		ecx, 2000
	xor		ax, ax
	mov		ah, dh
	mov		al, 0x20
	.l:
	mov		WORD[edi], ax
	add		edi, 2
	loop	.l
	mov		BYTE[cu_x], 0
	mov		BYTE[cu_y], 0
	jmp		complete32

complete32:
	call	updateCursor32
	popa
	ret

main32:
	pop		DWORD[s3dataLoc]
	cli

	lgdt	[gdt]
	lidt	[idt]

	;disable paging
	mov		eax, cr0
	and		eax, 0x7fffffff
	mov		cr0, eax

	mov		esi, DWORD[s3dataLoc]

	call	updateCursorMemData32

	cmp		DWORD[esi], 0x297f09ae
	je		.boot

	mov		dh, 0xc
	mov		esi, msgMFail
	call	println32
	cli
	hlt

	.boot:
	mov		dh, 0x7
	mov		esi, msgBoot
	call	printp32

	call	updateLoadingWheel32

	mov		esi, DWORD[s3dataLoc]
	mov		dx, WORD[esi + 12]
	mov		WORD[s3BMode], dx
	mov		edx, DWORD[esi + 14]
	mov		DWORD[s3jmpDest], edx
	mov		edx, DWORD[esi + 18]
	mov		DWORD[s3jmpDest + 4], edx
	mov		edx, DWORD[esi + 22]
	mov		DWORD[s3archFlags], edx

	mov		esi, DWORD[esi + 28] ; entry state
	cmp		esi, 0
	je		.no_entry_state
	mov		edi, s3entry_a
	mov		ecx, 44
	cld
	rep		movsb
	.no_entry_state:

	cmp		DWORD[s3entry_sp], 0
	jne		.entry_sp_set
	mov		DWORD[s3entry_sp], 0x7c00
	.entry_sp_set:
	cmp		DWORD[s3entry_bp], 0
	jne		.entry_bp_set
	mov		edx, DWORD[s3entry_sp]
	mov		DWORD[s3entry_bp], edx
	.entry_bp_set:

	mov		esi, DWORD[s3dataLoc]
	mov		ebx, DWORD[esi + 4]
	mov		ecx, DWORD[esi + 8]
	.parseMap:
	push	ecx
	mov		esi, DWORD[ebx + 8]
	cmp		esi, 0
	je		.parseMap_clear
	mov		edi, DWORD[ebx]
	mov		ecx, DWORD[ebx + 4]
	call	copyMem32
	jmp		.parseMap_c
	.parseMap_clear:
	mov		esi, DWORD[ebx]
	mov		ecx, DWORD[ebx + 4]
	call	clearMem32
	.parseMap_c:
	pop		ecx
	add		ebx, 12
	loop	.parseMap

	call	printNln32

	mov		dx, WORD[s3BMode]
	cmp		dx, 1
	je		realMode
	cmp		dx, 2
	je		protMode

	mov		dh, 0xc
	mov		esi, msgUType
	call	println32
	cli
	hlt

clearMem32: ; esi: start, ecx: length
	call	updateLoadingWheel32
	.cclear:
	;cmp		ecx, 0
	;je		.comp
	mov		BYTE[esi], 0
	inc		esi
	loop	.cclear
	.comp:
	ret

copyMem32: ; esi: source, ecx: length, edi: dest
	call	updateLoadingWheel32
	cmp		ecx, 0
	je		.comp
	push	edx
	xor		edx, edx
	.copy:
	mov		dl, BYTE[esi]
	mov		BYTE[edi], dl
	inc		esi
	inc		edi
	loop	.copy
	pop		edx
	.comp:
	ret

lastSec		db	0

wait1Sec:
	pusha
	call	getCMOSSec
	mov		BYTE[lastSec], al
	.w:
	call	getCMOSSec
	cmp		BYTE[lastSec], al
	je		.w
	popa
	ret

getCMOSSec:
	push	edx
	mov		dx, 0x70
	mov		al, 0
	out		dx, al
	mov		dx, 0x71
	xor		eax, eax
	in		al, dx
	pop		edx
	ret

s3end:
	push	esi
	push	edx
	mov		dh, 0xa
	mov		esi, msgBye
	call	println32
	call	wait1Sec
	mov		dh, 0x7
	call	clearScreen32
	pop		edx
	pop		esi
	ret

gdt_start:
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
gdt_end:

gdt:
	dw gdt_end - gdt_start - 1
	dq gdt_start


; ------------------- REAL MODE -------------------

jmpDest16	dw	0

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
	dd gdt16_start

idt16:
	dw 0x3ff
	dd 0

realMode:
	mov		dx, WORD[s3jmpDest]
	mov		WORD[jmpDest16], dx

	cli

	call	s3end

	lgdt	[gdt16]

	jmp		0x8:cont16

	cli
	hlt


; ------------------- REAL MODE CONT -------------------
bits 16

realModeInt	db	1

cont16: ; 16 bit protected mode
	cli

	mov		eax, 0x10
	mov		ds, eax
	mov		es, eax
	mov		fs, eax
	mov		gs, eax
	mov		ss, eax

	mov		eax, cr0
	and		eax, 0x7ffffffE
	mov		cr0, eax

	jmp		0x0:cont16b

cont16b: ; real real mode
	mov		ax, 0
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax
	mov		sp, 0x7c00

	lidt	[idt16]

	mov		edx, DWORD[s3archFlags]
	and		edx, 1
	cmp		edx, 0
	je		.intOn
	mov		BYTE[realModeInt], 0
	.intOn:

	mov		dx, WORD[jmpDest16]
	mov		WORD[.b16_jmp_far + 1], dx
	mov		dx, WORD[s3entry_cs]
	mov		WORD[.b16_jmp_far + 3], dx

	mov		edx, DWORD[s3entry_a]
	mov		DWORD[.b16_set_ax + 2], edx

	mov		ebx, DWORD[s3entry_b]
	mov		ecx, DWORD[s3entry_c]
	mov		edx, DWORD[s3entry_d]
	mov		esi, DWORD[s3entry_si]
	mov		edi, DWORD[s3entry_di]
	mov		esp, DWORD[s3entry_bp]
	mov		ebp, DWORD[s3entry_sp]

	cmp		BYTE[realModeInt], 1
	jne		.noInt
	sti
	.noInt:

	mov		ax, WORD[s3entry_ds]
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax

	.b16_set_ax:
	mov		eax, 0

	.b16_jmp_far:
	jmp		0:0


; ------------------- PROT MODE -------------------
bits 32

gdt32_alt:
	dw	0
	dq	0

protMode:
	call	s3end

	cmp		DWORD[s3entry_gdtb], 0
	je		.no_alt_gdt
	cmp		DWORD[s3entry_gdtl], 0
	je		.no_alt_gdt
	mov		edx, DWORD[s3entry_gdtb]
	mov		DWORD[gdt32_alt + 2], edx
	mov		dx, WORD[s3entry_gdtl]
	mov		WORD[gdt32_alt], dx
	lgdt	[gdt32_alt]

	push	dword WORD[s3entry_cs]
	push	.alt_gdt_set_cs
	retf
	.alt_gdt_set_cs:

	mov		ax, WORD[s3entry_ds]
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax

	.no_alt_gdt:

	mov		eax, DWORD[s3entry_a]
	mov		ebx, DWORD[s3entry_b]
	mov		ecx, DWORD[s3entry_c]
	mov		edx, DWORD[s3entry_d]
	mov		esi, DWORD[s3entry_si]
	mov		edi, DWORD[s3entry_di]
	mov		esp, DWORD[s3entry_bp]
	mov		ebp, DWORD[s3entry_sp]

	jmp		DWORD[s3jmpDest]

align 32, db 0xcc
