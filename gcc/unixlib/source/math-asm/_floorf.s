;----------------------------------------------------------------------------
;
; $Source: /usr/local/cvsroot/gccsdk/unixlib/source/math-asm/_floorf.s,v $
; $Date: 2002/11/26 21:52:07 $
; $Revision: 1.1 $
; $State: Exp $
; $Author: admin $
;
;----------------------------------------------------------------------------

	GET	clib/unixlib/asm_dec.s

	AREA	|C$$code|,CODE,READONLY

	EXPORT	floorf
	NAME	floorf
floorf
	STR	a1, [sp, #-4]!
	LDFS	f0, [sp], #4
	RNDSM	f0, f0
	return	AL, pc, lr

	END
