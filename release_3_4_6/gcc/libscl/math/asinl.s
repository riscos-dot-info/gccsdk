	AREA	|C$$code|,CODE,READONLY

	EXPORT	asinl
asinl
	STMFD	sp!, {a1, a2}
	LDFD	f0, [sp], #8
	ASND	f0, f0
	MOV	pc, lr

	END