;----------------------------------------------------------------------------
;
; $Source: /usr/local/cvsroot/unixlib/source/math-asm/s/_log,v $
; $Date: 2000/12/21 15:09:13 $
; $Revision: 1.2 $
; $State: Exp $
; $Author: admin $
;
;----------------------------------------------------------------------------

	GET	clib/unixlib/asm_dec.s

	AREA	|C$$code|,CODE,READONLY

	EXPORT	log
log
	STMFD	sp!, {a1, a2}
	LDFD	f0, [sp], #8
	LGND	f0, f0
	return	AL, pc, lr

	END
