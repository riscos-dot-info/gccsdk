;----------------------------------------------------------------------------
;
; $Source: /usr/local/cvsroot/gccsdk/unixlib/source/math-asm/_fegetrnd.s,v $
; $Date: 2001/01/29 15:10:20 $
; $Revision: 1.2 $
; $State: Exp $
; $Author: admin $
;
;----------------------------------------------------------------------------

	GET	clib/unixlib/asm_dec.s

	AREA	|C$$code|, CODE, READONLY

	EXPORT	fegetround
	NAME	fegetround
fegetround
	; The default rounding mode cannot be set on the ARM FPU without
	; handcoding the rounding mode into the instruction. We therefore
	; always default to round-to-nearest.
	mov	a1, #0
	return	AL, pc, lr

	END
