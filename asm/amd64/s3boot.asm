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

bits 64
org 0x5000

; VERSION 4

jmp main64

%define ENTRY_STATE_SIZE 84
; entry state
s3entry_a	dq	0
s3entry_b	dq	0
s3entry_c	dq	0
s3entry_d	dq	0
s3entry_sp	dq	0
s3entry_bp	dq	0
s3entry_si	dq	0
s3entry_di	dq	0
s3entry_gdtb	dq	0
s3entry_gdtl	dq	0
s3entry_cs	dw	0
s3entry_ds	dw	0


main64:
	pop		QWORD[s3dataLoc]
	cli

	lgdt	[gdt]
	lidt	[idt]

	bits 32

	;disable paging
	mov		eax, cr0
	and		eax, 0x7fffffff
	mov		cr0, eax

	bits 16
	pushf
	push	word 0x8
	push	word .b1
	iret

	.b1:

	bits 32
	;disable long mode
	mov		ecx, 0xc0000080
	rdmsr
	;LM bit
	and		eax, 0xfffffeff
	wrmsr

	mov		eax, cr4
	;PAE bit
	and		eax, 0xffffffdf
	mov		cr4, eax

	mov		eax, cr0
	or		eax, 1
	mov		cr0, eax

	jmp		s3boot_common_main


%include "asm/x86/s3boot_common.asm"
