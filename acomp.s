section .text
	global checkerboard24

checkerboard24:
	push 	ebp
	mov 	ebp, esp
	sub	esp, 12

	%define sqs  	[ebp+12]	
	%define color1	[ebp+16]
	%define color2	[ebp+20]

	%define padsize [ebp-4]
	%define width 	[ebp-8]
	%define height 	[ebp-12]

	push    ebx	;[ebp-16]
	push    esi	;[ebp-20]
	push    edi	;[ebp-24]

;calculate padding

	mov	esi, [ebp+8]	;imgInfo
	mov	eax, [esi]	;imgInfo->width
	lea	eax, [eax+2*eax];multiply by 3
	mov	ebx, 4
	xor	edx, edx
	div 	ebx		;eax=edx:eax/ebx, edx=r
	test	edx, edx
	jz	pad
	sub	edx, 4
	neg	edx
pad:	mov	padsize, edx
	mov	ebx, [esi+8]	;pixel data - ebx
	mov	eax, [esi]
	mov	width, eax
	mov	eax, [esi+4]
	mov	height, eax

new_row:
	mov 	edx, color1	;c1
	mov	edi, color2	;c2
	mov	[ebp+16], edi	;c2 becomes c1
	mov 	color2, edx	;c1 becomes c2
	mov 	eax, sqs	;eax - square height counter set
new_line:
	mov	edx, color1	; edx - stores primary color of the row
	mov	edi, color2	; edi - stores secondary color of the row
	mov 	ecx, width	; exc - width counter set
	mov 	esi, sqs	; esi - square width counter set
new_pixel:
	mov 	[ebx], dx	;start with blue, then green, then red
	ror 	edx, 8
	mov 	[ebx+2], dh
	rol 	edx, 8
	add	ebx, 3

;checks
	dec	ecx 	;test width
	dec	esi	;test square width
	jecxz	padding
	jnz	new_pixel
	xchg 	edx, edi
	mov 	esi, sqs
	jmp	new_pixel
	
	
padding:
	add	ebx, padsize
	

;checks
	dec 	dword height		;dec height
	jz 	exit

	dec	eax	;test square height
	jz	new_row
	jmp	new_line

exit:
	pop	edi
	pop	esi
	pop	ebx

	mov	esp, ebp
	pop	ebp
	ret
