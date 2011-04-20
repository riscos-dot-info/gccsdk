;----------------------------------------------------------------------------
;
; $Source: /usr/local/cvsroot/gccsdk/unixlib/source/math-asm/_sinhf.s,v $
; $Date: 2002/11/26 21:52:07 $
; $Revision: 1.1 $
; $State: Exp $
; $Author: admin $
;
;----------------------------------------------------------------------------

	GET	clib/unixlib/asm_dec.s

	AREA	|C$$code|,CODE,READONLY

	; return (exp(x) - exp(-x)) / 2
	EXPORT	sinhf
	NAME	sinhf
sinhf
	STR	a1, [sp, #-4]!
	LDFS	f0, [sp], #4
	MNFS	f1, f0
	EXPS	f0, f0
	EXPS	f1, f1
	SUFS	f0, f0, f1
	DVFS	f0, f0, #2
	return	AL, pc, lr

	END