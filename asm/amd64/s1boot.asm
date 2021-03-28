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
;0x7e00 - 0x7fff   sGboot
;0x8000 - 0x8fff   temp/FAT entries for loading s2boot
;0x00009000 - 0x0003ffff   s2boot + bdd.so unparsed
;0x00040000 - 0x0006ffff   s2boot
;0x00070000 - 0x00072fff   bios memory map
;0x00073000 - 0x0007ffff   s2boot map
;0x00076000 - 0x00079fff   page mappings

;------------------------------------------------------------
;     MAIN STARTUP AREA
;------------------------------------------------------------

%include "asm/x86/s1boot_common.asm"


;------------------------------------------------------------
;     BOOT TO 32
;------------------------------------------------------------

bits 16

msg32no64		db	"Your computer does not support 64 bit", 0

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


longModeAvailable:
	push	eax
	push	edx
	mov		eax, 0x80000000
	cpuid
	cmp		eax, 0x80000001
	jb		.no64

	mov		eax, 0x80000001
	cpuid
	test	edx, 0x20000000
	jz		.no64
	clc
	jmp		.comp
	.no64:
	stc
	.comp:
	pop		edx
	pop		eax
	ret

bootTo32:
	call	longModeAvailable
	jc		.no64

	call	resetExceptionHandlers16

	cli
	lgdt	[gdt32]
	mov		eax, cr0
	or		eax, 1
	mov		cr0, eax

	jmp		0x8:bootTo64

	.no64:
	mov		dh, 0x4
	mov		si, msg32no64
	call	println16
	jmp		.error
	.error:
	cli
	hlt




;------------------------------------------------------------
;     BOOT TO 64
;------------------------------------------------------------

bits 32

gdt64_start:
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
	db 10101111b
	db 00000000b
	
	;data
	db 11111111b
	db 11111111b
	db 00000000b
	db 00000000b
	db 00000000b
	db 10010010b
	db 10001111b
	db 00000000b
gdt64_end:

gdt64:
	dw gdt64_end - gdt64_start - 1
	dq gdt64_start


idt64_start:
	; div by 0
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; ?
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0
	
	; nmi
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0
	
	; breakpoint
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; overflow
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; bounds range exception
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; inv opcode
	dw invalidOpcode64
	dd 0x8e000008
	dw 0
	dd 0
	dd 0

	; device not available
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; double fault
	dw doubleFault64
	dd 0x8e000008
	dw 0
	dd 0
	dd 0

	; coprocessor seg overrun
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; invalid tss
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; segment not present
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; stack-segment fault
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; general protection fault
	dw generalPF64
	dd 0x8e000008
	dw 0
	dd 0
	dd 0

	; page fault
	dw pageFault64
	dd 0x8e000008
	dw 0
	dd 0
	dd 0

	; ?2
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; x87 fpu error
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; alignment check
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; machine check
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0

	; simd fp exception
	dw unknownException64
	dd 0x0e000008
	dw 0
	dd 0
	dd 0
idt64_end:

idt64:
	dw idt64_end - idt64_start - 1
	dq idt64_start


; 1111111111111111111111111111111111111111111111111111111111111111
; |reserved      ||os pl4t||in pdpt||in pd  ||in pt  ||in page   |

; map (phys) 0x0000000000000000 - 0x0000000000400000 to (virt) 0x0000000000000000
; 0x00076000 - 0x00077000: page map level 4 table
; 0x00077000 - 0x00078000: page directory page table
; 0x00078000 - 0x00079000: page directory table
; 0x00079000 - 0x0007a000: page table for first mapping

%define pageTablesBase 0x76000

setupPaging:
	push	eax
	push	ecx
	push	edx
	push	edi

	mov		edi, pageTablesBase
	xor		eax, eax
	mov		ecx, 0x4000
	rep		stosb

	; page map level 4 table
	mov		DWORD[pageTablesBase], pageTablesBase + 0x1003 ; phys address 0x1000, set present (0x1) and r/w (0x2) bit

	; page directory page table
	mov		DWORD[pageTablesBase + 0x1000], pageTablesBase + 0x2003 ; phys address 0x2000, set present (0x1) and r/w (0x2) bit

	; page directory table
	mov		DWORD[pageTablesBase + 0x2000], pageTablesBase + 0x3003 ; phys address 0x3000, set present (0x1) and r/w (0x2) bit

	; page tables
	mov		ecx, 512
	mov		edi, pageTablesBase + 0x3000
	mov		edx, 0x3
	.writePT:
	mov		DWORD[edi], edx
	add		edi, 8
	add		edx, 0x1000
	loop	.writePT

	mov		eax, pageTablesBase
	mov		cr3, eax

	pop		edi
	pop		edx
	pop		ecx
	pop		eax
	ret

longModeSetup:
	call	setupPaging

	mov		eax, cr4
	;PAE bit
	or		eax, 0x20
	mov		cr4, eax

	;enable long mode
	mov		ecx, 0xc0000080
	rdmsr
	;LM bit
	or		eax, 0x100
	wrmsr

	;enable paging
	mov		eax, cr0
	or		eax, 0x80000000
	mov		cr0, eax

	lgdt	[gdt64]
	ret

bootTo64:
	mov		ax, 0x10
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax
	mov		esp, 0x7c00

	call	longModeSetup

	jmp		0x8:boot64




bits 64

msgELFS2		db	"Parsing ELF ", 0
msgInvELF		db	"Invalid ELF format in s2boot image", 0
msgInvBitS2		db	"s2boot image was compiled for 32bit, expected 64bit", 0
msgInvTpS2		db	"Expected type 2 (executable) or type 3 (dynamic) for s2boot image", 0
msgInvISS2		db	"s2boot image was not compiled for x86_64", 0
msgNumSkipSeg1	db	"Skipped ", 0
msgNumSkipSeg2	db	" invalid segments", 0
msgNumSeg1		db	"Loaded ", 0
msgNumSeg2		db	" segments", 0
msgNoSeg		db	"Loading failed", 0
msgS2			db	"Calling s2boot", 0
msgS2Return		db	"s2boot returned: ", 0

s2bootDynSeg	dq	0
s2bootDynSegLen	dq	0
s2bootRelaBase	dq	0
s2bootRelaLen	dq	0
skipSegments	db	0
numSegments		db	0
s2bootEntry		dq	0

dec_out			db	"0000000000000000000000", 0
hex_out			db	"0x0000000000000000 ", 0

msgErrCP		db	"err=", 0
msgErrCP2		db	"rip=", 0
errCode			dq	0
errRIP			dq	0


print64: ;pointer in rsi, attributes in dh
	push	rax
	push	rdx
	.l:
	lodsb
	cmp		al, 0
	je		.c
	mov		dl, al
	call	printChar64
	jmp		.l
	.c:
	call	updateCursor64
	pop		rdx
	pop		rax
	ret

printp64: ;pointer in rsi, attributes in dh
	push	rsi
	mov		rsi, msgPre
	call	print64
	pop		rsi
	call	print64
	ret

println64:
	push	rsi
	mov		rsi, msgPre
	call	print64
	pop		rsi
	call	print64
	call	printNln64
	ret

printlns64:
	call	print64
	call	printNln64
	ret

printChar64: ;data in dx dl-char dh-attr
	push	rax
	push	rcx
	push	rsi
	push	rdi
	
	mov		rdi, 0xb8000
	xor		rax, rax
	xor		rcx, rcx
	mov		rsi, cu_x
	mov		al, BYTE[rsi]
	mov		cl, 2
	mul		cl
	add		rdi, rax
	mov		rsi, cu_y
	mov		al, BYTE[rsi]
	mov		cl, 160
	mul		cl
	add		rdi, rax

	cmp		dl, 0xa
	je		.nl

	mov		rsi, cu_x
	mov		WORD[rdi], dx
	inc		BYTE[rsi]
	cmp		BYTE[rsi], 80
	jb		.c
	.nl:
	call	printNln64
	.c:
	call	updateCursor64
	pop		rdi
	pop		rsi
	pop		rcx
	pop		rax
	ret

printNln64:
	push	rsi
	mov		rsi, cu_x
	mov		BYTE[rsi], 0
	mov		rsi, cu_y
	inc		BYTE[rsi]
	cmp		BYTE[rsi], 25
	jne		.c
	call	shiftUp64
	.c:
	call	updateCursor64
	pop		rsi
	ret

shiftUp64:
	push	rcx
	push	rdx
	push	rsi
	push	rdi
	mov		rsi, cu_y
	dec		BYTE[rsi]
	mov		rdi, 0xb8000
	mov		rcx, 960
	.l:
	mov		edx, DWORD[rdi+160]
	mov		DWORD[rdi], edx
	add		rdi, 4
	loop	.l
	mov		rcx, 40
	.l2:
	mov		DWORD[rdi], 0x07200720
	add		rdi, 4
	loop	.l2
	call	updateCursor64
	pop		rdi
	pop		rsi
	pop		rdx
	pop		rcx
	ret

updateCursor64:
	push	rax
	push	rbx
	push	rcx
	push	rdx
	push	rsi
	
	xor		rax, rax
	mov		rsi, cu_y
	mov		al, BYTE[rsi]
	mov		cl, 80
	mul		cl
	mov		rsi, cu_x
	add		al, BYTE[rsi]
	mov		rbx, rax

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

	pop		rsi
	pop		rdx
	pop		rcx
	pop		rbx
	pop		rax
	ret

updateCursorMemData64:
	push	rax
	push	rbx
	push	rcx
	push	rdx
	push	rsi
	
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
	mov		rsi, cu_x
	mov		BYTE[rsi], dl
	mov		rsi, cu_y
	mov		BYTE[rsi], al
	
	pop		rsi
	pop		rdx
	pop		rcx
	pop		rbx
	pop		rax
	ret

updateLoadingWheel64:
	push	rax
	push	rcx
	push	rdx
	push	rsi
	push	rdi

	mov		rdi, 0xb8000
	xor		rax, rax
	xor		rcx, rcx
	mov		rsi, cu_x
	mov		al, BYTE[rsi]
	mov		cl, 2
	mul		cl
	add		rdi, rax
	mov		rsi, cu_y
	mov		al, BYTE[rsi]
	mov		cl, 160
	mul		cl
	add		rdi, rax

	mov		dx, WORD[rdi]
	cmp		dl, '-'
	je		.minus
	cmp		dl, '/'
	je		.slash
	cmp		dl, 0x5c
	je		.back
	cmp		dl, '|'
	je		.line
	mov		dl, '-'
	jmp		.c
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
	mov		WORD[rdi], dx
	pop		rdi
	pop		rsi
	pop		rdx
	pop		rcx
	pop		rax
	ret

printDec64: ; number in rdx
	push	rax
	push	rbx
	push	rdx
	push	rsi
	push	rdi
	
	mov		rdi, dec_out + 21
	.reset:
	mov		BYTE[rdi], 0x30
	dec		rdi
	mov		rsi, dec_out - 1
	cmp		rdi, rsi
	jne		.reset
	mov		rdi, dec_out + 21
	mov		rbx, 10
	mov		rax, rdx
	.loop:
	xor		rdx, rdx
	div		rbx
	add		dl, 48
	mov		BYTE[rdi], dl
	cmp		rax, 0
	je		.print
	dec		rdi
	jmp		.loop
	.print:
	mov		rsi, dec_out - 1
	.cut:
	inc		rsi
	cmp		BYTE[rsi], 0x30
	je		.cut
	mov		rdi, dec_out + 22
	cmp		rsi, rdi
	jne		.p
	mov		rsi, dec_out + 21
	.p:
	mov		dh, 00000101b
	call	print64
	
	pop		rdi
	pop		rsi
	pop		rdx
	pop		rbx
	pop		rax
	ret

printHex64:
	push	rax
	push	rbx
	push	rcx
	push	rdx
	push	rsi

	mov		rcx, 16
	.char_loop:
	mov		rax, rdx
	shr		rdx, 4
	and		rax, 0xf

	mov		rbx, hex_out
	add		rbx, 1
	add		rbx, rcx

	mov		BYTE[rbx], 0x30

	cmp		ax, 0xa
	jl		.set_letter
	add		byte[rbx], 7
	jl		.set_letter
	.set_letter:
	add		byte[rbx],al 
	loop	.char_loop
	jmp		.print_hex_done
	.print_hex_done:
	mov		rcx, 16
	mov		rsi, hex_out
	add		rsi, 2
	.cmp:
	cmp		BYTE[rsi], 0x30
	jne		.p
	inc		rsi
	loop	.cmp
	;jmp		.ret
	.p:
	mov		rsi, hex_out
	mov		dh, 00001101b
	call	print64
	.ret:
	pop		rsi
	pop		rdx
	pop		rcx
	pop		rbx
	pop		rax
	ret

invalidOpcode64:
	pop		QWORD[errRIP]
	mov		rsi, msgInvOp
	jmp		exception64

doubleFault64:
	pop		QWORD[errCode]
	pop		QWORD[errRIP]
	mov		rsi, msgDoubleF
	jmp		exception64

generalPF64:
	pop		QWORD[errCode]
	pop		QWORD[errRIP]
	mov		rsi, msgGPF
	jmp		exception64

pageFault64:
	pop		QWORD[errCode]
	pop		QWORD[errRIP]
	mov		rsi, msgPF
	jmp		exception64

unknownException64: ; this is unused
	pop		QWORD[errCode]
	pop		QWORD[errRIP]
	mov		rsi, msgUnExcept
	jmp		exception64

exception64:
	call	updateCursorMemData64
	mov		dh, 0x4
	call	println64
	call	printErrC64
	mov		dh, 0x4
	mov		rsi, msgExcept
	call	println64
	cli
	hlt

printErrC64:
	mov		dh, 0xf
	mov		rsi, msgErrCP
	call	printp64
	mov		rdx, QWORD[errCode]
	call	printHex64
	mov		dh, 0xf
	mov		rsi, msgErrCP2
	call	print64
	mov		rdx, QWORD[errRIP]
	call	printHex64
	call	printNln64
	ret

parseELF64:
	call	updateLoadingWheel64
	cmp		DWORD[s2bootFileLoad], 0x464c457f
	jne		.invalid_format
	cmp		BYTE[s2bootFileLoad + 0x04], 0x02
	jne		.invalid_bit
	cmp		WORD[s2bootFileLoad + 0x05], 0x0101
	jne		.invalid_format
	cmp		WORD[s2bootFileLoad + 0x10], 0x0002
	je		.valid_type
	cmp		WORD[s2bootFileLoad + 0x10], 0x0003
	je		.valid_type
	jmp		.invalid_type
	.valid_type:
	cmp		WORD[s2bootFileLoad + 0x12], 0x003E
	jne		.invalid_instruction_set
	call	updateLoadingWheel64
	mov		QWORD[s2mapStart], s2bootMapLocation
	mov		QWORD[s2bootBase], s2bootStartVirt
	movzx	rcx, WORD[s2bootFileLoad + 0x38] ; number of ph table entries
	mov		rbx, s2bootFileLoad
	add		rbx, QWORD[s2bootFileLoad + 0x20] ; start of ph table
	.parse_program_header:
	call	updateLoadingWheel64
	push	rbx
	push	rcx
	cmp		DWORD[rbx], 1 ; loadable segment
	jne		.check_dyn
	mov		rax, QWORD[rbx + 0x10] ; target address
	add		rax, QWORD[rbx + 0x28] ; size in memory
	cmp		rax, s2bootMaxSize ; check if target address + size is below maximum size
	jb		.segm_c_2
	inc		BYTE[skipSegments]
	jmp		.parse_program_header_f
	.segm_c_2:
	; add s2map entry
	mov		rax, 16
	mul		WORD[s2mapLength]
	add		rax, QWORD[s2mapStart]
	mov		rdi, rax
	mov		rdx, QWORD[rbx + 0x10] ; target address
	add		rdx, s2bootStartVirt
	mov		QWORD[rdi], rdx
	add		rdx, QWORD[rbx + 0x28] ; size in memory
	mov		QWORD[rdi + 8], rdx
	inc		WORD[s2mapLength]

	mov		rdi, QWORD[rbx + 0x10] ; target address
	add		rdi, s2bootStartVirt
	mov		rcx, QWORD[rbx + 0x28] ; size in memory
	xor		rax, rax
	rep		stosb

	mov		rcx, QWORD[rbx + 0x20] ; size in file
	mov		rdi, QWORD[rbx + 0x10] ; target address
	add		rdi, s2bootStartVirt
	mov		rsi, QWORD[rbx + 0x08] ; offset in file
	add		rsi, s2bootFileLoad
	cld
	.copy_segm:
	call	updateLoadingWheel64
	movsb
	loop	.copy_segm
	inc		BYTE[numSegments]
	jmp		.parse_program_header_f
	.check_dyn:
	cmp		DWORD[rbx], 2 ; dynamic linker segment
	jne		.parse_program_header_f
	mov		rdx, QWORD[rbx + 0x08]
	mov		QWORD[s2bootDynSeg], rdx
	mov		rdx, QWORD[rbx + 0x20]
	mov		QWORD[s2bootDynSegLen], rdx
	.parse_program_header_f:
	pop		rcx
	pop		rbx
	movzx	rdx, WORD[s2bootFileLoad + 0x36] ; size of ph entry
	add		rbx, rdx
	dec		rcx
	jnz		.parse_program_header

	call	updateLoadingWheel64
	mov		rdx, QWORD[s2bootFileLoad + 0x18] ; entry point address
	add		rdx, s2bootStartVirt
	mov		QWORD[s2bootEntry], rdx

	cmp		QWORD[s2bootDynSeg], 0
	je		.link_f
	; dynamic link
	call	updateLoadingWheel64
	mov		rax, QWORD[s2bootDynSegLen]
	xor		rdx, rdx
	mov		rdi, 16
	div		rdi ; length of entry
	mov		rcx, rax
	mov		rbx, QWORD[s2bootDynSeg]
	add		rbx, s2bootFileLoad
	.parseDynSec:
	call	updateLoadingWheel64
	cmp		QWORD[rbx], 7 ; rela section
	je		.parseDynSec_relaBase
	cmp		QWORD[rbx], 8 ; rela section length
	je		.parseDynSec_relaLen
	jmp		.parseDynSec_f
	.parseDynSec_relaBase:
	mov		rdx, QWORD[rbx + 8]
	mov		QWORD[s2bootRelaBase], rdx
	jmp		.parseDynSec_f
	.parseDynSec_relaLen:
	mov		rdx, QWORD[rbx + 8]
	mov		QWORD[s2bootRelaLen], rdx
	jmp		.parseDynSec_f
	.parseDynSec_f:
	add		rbx, 16
	loop	.parseDynSec

	cmp		QWORD[s2bootRelaBase], 0
	je		.link_f
	call	updateLoadingWheel64
	mov		rax, QWORD[s2bootRelaLen]
	xor		rdx, rdx
	mov		rdi, 24
	div		rdi ; length of entry in rela section
	mov		rcx, rax
	mov		rbx, QWORD[s2bootRelaBase]
	add		rbx, s2bootStartVirt
	.setRela:
	push	rbx
	xor		rdx, rdx
	mov		edx, DWORD[rbx + 8]
	cmp		rdx, 8 ; type DYNL_R_AMD64_RELATIVE (base + addend)
	jne		.setRela_f
	mov		rdx, QWORD[rbx + 16] ; addend
	add		rdx, s2bootStartVirt
	mov		rbx, QWORD[rbx]
	add		rbx, s2bootStartVirt
	mov		QWORD[rbx], rdx
	.setRela_f:
	pop		rbx
	add		rbx, 24
	loop	.setRela

	.link_f:

	call	printNln64
	cmp		BYTE[skipSegments], 0
	jbe		.print_loaded_segs
	mov		dh, 0xe
	mov		rsi, msgNumSkipSeg1
	call	printp64
	movzx	rdx, BYTE[skipSegments]
	call	printDec64
	mov		dh, 0xe
	mov		rsi, msgNumSkipSeg2
	call	print64
	call	printNln64
	.print_loaded_segs:
	mov		dh, 0x7
	mov		rsi, msgNumSeg1
	call	printp64
	movzx	rdx, BYTE[numSegments]
	call	printDec64
	mov		dh, 0x7
	mov		rsi, msgNumSeg2
	call	print64
	call	printNln64
	cmp		BYTE[numSegments], 1
	jb		.no_segments ; 0 loaded segments, something definitely went wrong
	clc
	ret
	.invalid_format:
	mov		rsi, msgInvELF
	jmp		.parse_error
	.invalid_bit:
	mov		rsi, msgInvBitS2
	jmp		.parse_error
	.invalid_type:
	mov		rsi, msgInvTpS2
	jmp		.parse_error
	.invalid_instruction_set:
	mov		rsi, msgInvISS2
	jmp		.parse_error
	.no_segments:
	mov		rsi, msgNoSeg
	jmp		.parse_error
	.parse_error:
	call	printNln64
	mov		dh, 0xc
	call	println64
	stc
	ret



boot64:
	mov		ax, 0x10
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		ss, ax
	mov		esp, 0x7c00

	lidt	[idt64]

	mov		dh, 0x7
	mov		rsi, msgELFS2
	call	printp64
	call	parseELF64
	jnc		.c1
	cli
	hlt

	.c1:

	mov		DWORD[membase], 0

	mov		dh, 0x7
	mov		rsi, msgS2
	call	println64

	mov		rdi, header_start
	mov		rbp, 0
	call	QWORD[s2bootEntry]

	.halt:
	cli
	hlt
	jmp		.halt




; SERVICE CALLBACK

servMemBuf	dq	0

reqBufStruct	dq	0
; videomode
reqWidth	dq	0
reqHeight	dq	0
reqBPP		dq	0
reqMode		dq	0
reqResFramebuffer	dq	0
reqResBytesPerLine	dq	0
curVidMode	dw	0
; read drive
reqDIndex	dq	0
reqSectorCount	dq	0
reqDest		dq	0
reqLBA		dq	0
; drive info
reqSecCountDest	dq	0
reqSecSizeDest	dq	0

dInfoTemp:
dInfoBufSize	dw	0x1e
dInfoInfoFlags	dw	0
dInfoCylinders	dd	0
dInfoHeads		dd	0
dInfoSecsPTrack	dd	0
dInfoSectors	dq	0
dInfoBytesPSec	dw	0
dInfoPEDD		dd	0

gdt64temp:
	dw 0
	dq 0
idt64temp:
	dw 0
	dq 0
stack64temp	dq	0

retCode		dq	0

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

srvModeSwitchAddr	dw	0

firmwareDriveName	db	"bios", 0

; services: (<di> - <description> (<up to 3 passed arguments>))
;  1 - upstream pre init (address to kmalloc)
;  2 - upstream init (<no args>)
;  10 - set video to closest (modeInfoStruct)
serviceCallback: ; di, si, dx, cx
	push	rbp
	mov		rbp, rsp
	mov		rax, 0
	cmp		rdi, 1
	je		.uPreInit
	cmp		rdi, 2
	je		.uInit
	cmp		rdi, 10
	je		.setVideo
	cmp		rdi, 20
	je		.firmwareDriveGetName
	cmp		rdi, 21
	je		.firmwareDriveRead
	cmp		rdi, 22
	je		.firmwareDriveInfo
	mov		rax, 1
	jmp		.end


	.uPreInit:
	mov		rdi, 0x1000
	call	rsi
	mov		QWORD[servMemBuf], rax
	cmp		QWORD[servMemBuf], 0
	je		.uPreInit_err
	cmp		QWORD[servMemBuf], 0xffff
	ja		.uPreInit_err
	mov		rax, 0
	jmp		.end
	.uPreInit_err:
	mov		rax, 27 ; TSX_MEMORY_RESERVED
	jmp		.end


	.uInit:
	jmp		.end


	.setVideo:
	mov		QWORD[reqBufStruct], rsi
	mov		rdx, QWORD[rsi]
	mov		QWORD[reqWidth], rdx
	mov		rdx, QWORD[rsi + 8]
	mov		QWORD[reqHeight], rdx
	mov		rdx, QWORD[rsi + 16]
	mov		QWORD[reqBPP], rdx
	mov		rdx, QWORD[rsi + 24]
	mov		QWORD[reqMode], rdx


	mov		WORD[srvModeSwitchAddr], .vbe_to16ret
	jmp		to16
	.vbe_to16ret:
	bits 16


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
	mov		WORD[retCode], 0
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


	mov		WORD[srvModeSwitchAddr], .vbe_from16ret
	jmp		from16
	.vbe_from16ret:
	bits 64


	mov		rsi, QWORD[reqBufStruct]
	mov		rdx, QWORD[reqResFramebuffer]
	mov		QWORD[rsi + 32], rdx ; write framebuffer address to struct
	mov		rdx, QWORD[reqResBytesPerLine]
	mov		QWORD[rsi + 40], rdx ; write framebuffer address to struct

	mov		rax, QWORD[retCode]
	jmp		.end


	.firmwareDriveGetName:
	mov		QWORD[rsi], firmwareDriveName
	mov		rax, 0
	jmp		.end


	.firmwareDriveRead:
	mov		QWORD[reqBufStruct], rsi
	mov		rdx, QWORD[rsi]
	mov		QWORD[reqDIndex], rdx
	mov		rdx, QWORD[rsi + 8]
	mov		QWORD[reqSectorCount], rdx
	mov		rdx, QWORD[rsi + 16]
	mov		QWORD[reqDest], rdx
	mov		rdx, QWORD[rsi + 24]
	mov		QWORD[reqLBA], rdx

	cmp		QWORD[reqDIndex], 0x7f
	jbe		.read_validD
	mov		QWORD[retCode], 13 ; TSX_NO_DEVICE
	jmp		.read_end
	.read_validD:

	mov		rcx, QWORD[reqSectorCount]
	.readLoop:
	push	rcx


	mov		WORD[srvModeSwitchAddr], .read_to16ret
	jmp		to16
	.read_to16ret:
	bits 16


	mov		edx, DWORD[reqLBA]
	mov		DWORD[dap_start_l], edx
	mov		edx, DWORD[reqLBA + 4]
	mov		DWORD[dap_start_h], edx

	mov		WORD[dap_count], 1

	mov		bx, WORD[servMemBuf]
	mov		WORD[dap_buf_off], bx
	mov		WORD[dap_buf_seg], 0

	mov		ah, 0x42
	mov		dl, BYTE[reqDIndex]
	add		dl, 0x80
	mov		si, dap_start
	int		0x13
	jc		.read_err

	mov		WORD[retCode], 0
	jmp		.read_done
	.read_err:
	mov		WORD[retCode], 19 ; TSX_IO_ERROR
	.read_done:


	mov		WORD[srvModeSwitchAddr], .read_from16ret
	jmp		from16
	.read_from16ret:
	bits 64


	pop		rcx
	cmp		WORD[retCode], 0
	jne		.read_end
	inc		QWORD[reqLBA]
	push	rcx
	mov		rdi, QWORD[reqDest]
	mov		rsi, QWORD[servMemBuf]
	mov		rcx, 512
	cld
	rep		movsb
	pop		rcx
	add		QWORD[reqDest], 512
	dec		rcx
	jnz		.readLoop


	.read_end:
	mov		rax, QWORD[retCode]
	jmp		.end


	.firmwareDriveInfo:
	mov		QWORD[reqDIndex], rsi
	mov		QWORD[reqSecCountDest], rdx
	mov		QWORD[reqSecSizeDest], rcx

	cmp		QWORD[reqDIndex], 0x7f
	jbe		.driveInfo_validD
	mov		QWORD[retCode], 13 ; TSX_NO_DEVICE
	jmp		.driveInfo_end
	.driveInfo_validD:


	mov		WORD[srvModeSwitchAddr], .driveInfo_to16ret
	jmp		to16
	.driveInfo_to16ret:
	bits 16


	mov		ah, 0x48
	mov		dl, BYTE[reqDIndex]
	add		dl, 0x80
	mov		si, dInfoTemp
	int		0x13
	jc		.driveInfo_err

	mov		WORD[retCode], 0
	jmp		.driveInfo_done
	.driveInfo_err:
	mov		WORD[retCode], 19 ; TSX_IO_ERROR
	.driveInfo_done:


	mov		WORD[srvModeSwitchAddr], .driveInfo_from16ret
	jmp		from16
	.driveInfo_from16ret:
	bits 64


	cmp		QWORD[retCode], 0
	jne		.driveInfo_err2
	mov		rdi, QWORD[reqSecCountDest]
	mov		rdx, QWORD[dInfoSectors]
	mov		QWORD[rdi], rdx
	mov		rdi, QWORD[reqSecSizeDest]
	movzx	rdx, WORD[dInfoBytesPSec]
	mov		QWORD[rdi], rdx
	.driveInfo_err2:


	.driveInfo_end:
	mov		rax, QWORD[retCode]
	jmp		.end


	.end:
	pop		rbp
	ret


to16:
	bits 64
	cli
	sgdt	[gdt64temp]
	sidt	[idt64temp]
	mov		QWORD[stack64temp], rsp
	mov		rsp, 0x7b00 ; set this in case the stack got relocated somewhere above 0xffff

	lgdt	[gdt32]
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

	mov		ax, WORD[srvModeSwitchAddr]
	jmp		ax

from16:
	bits 16
	lgdt	[gdt32]
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

	mov		eax, cr4
	;PAE bit
	or		eax, 0x20
	mov		cr4, eax
	;enable long mode
	mov		ecx, 0xc0000080
	rdmsr
	;LM bit
	or		eax, 0x100
	wrmsr
	;enable paging
	mov		eax, cr0
	or		eax, 0x80000000
	mov		cr0, eax
	lgdt	[gdt64temp]
	jmp		0x8:.64bitlong
	bits	64
	.64bitlong:
	mov		rsp, QWORD[stack64temp]
	lidt	[idt64temp]

	movzx	rax, WORD[srvModeSwitchAddr]
	jmp		rax









align 32, db 0xcc

s1bootEndAddr:
