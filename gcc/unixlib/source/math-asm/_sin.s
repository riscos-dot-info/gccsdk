;----------------------------------------------------------------------------
;
; $Source: /usr/local/cvsroot/gccsdk/unixlib/source/math-asm/_sin.s,v $
; $Date: 2002/09/24 21:02:37 $
; $Revision: 1.3 $
; $State: Exp $
; $Author: admin $
;
;----------------------------------------------------------------------------

	GET	clib/unixlib/asm_dec.s

	AREA	|C$$code|,CODE,READONLY

	EXPORT	sin
	EXPORT	sinl
	NAME	sin
sin
sinl
	STMFD	sp!, {a1, a2}
	LDFD	f0, [sp], #8
	SIND	f0, f0
	return	AL, pc, lr

	END
