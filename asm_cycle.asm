;; file: asm_cycle.asm
	;; To create executable:
	;; nasm -f elf asm_cycle.asm
	;; gcc -m32 -o lfsr lfsr.o driver.c asm_io.o

	%include "asm_io.inc"

	segment	.data
	;;
	;; initialized data is put in the data segment here
	;;

	seed dd 0x00cafeba

	segment	.bss
	;;
	;; uninitialized data is put in the bss segment
	;;
	lfsr	resd	1

	global asm_cycle
	segment	.text
asm_cycle:
	mov	eax, [esp + 4]
	push	eax
	mov	edx, [eax]
	xor	eax, eax		; clear out eax

	xor	edi, edi		; bit
	mov	esi, edx		; copy register
	shr	esi, 0
	mov	edi, esi		 ; bit = lfsr >> 0

	mov	esi, edx
	shr	esi, 1
	xor	edi, esi		 ; bit = bit ^ (lfsr >> 1)

	mov	esi, edx
	shr	esi, 3
	xor	edi, esi		; bit = bit ^ (lfsr >> 3)

	mov	esi, edx
	shr	esi, 4
	xor	edi, esi		 ; bit = bit ^ (lfsr >> 4)
	and	edi, 0x1                 ; keep only lsb

	shr	edx, 1			 ; shift register right
	shl	edi, 23			 ; shift bit to msb
	or	edx, edi		 ; set msb to bit

	pop	ecx
	mov	[ecx], edx
	mov	eax, edx
	ret
