;
; Copyright (C) 2022 user94729 and contributors
;
; This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
; If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
;

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

