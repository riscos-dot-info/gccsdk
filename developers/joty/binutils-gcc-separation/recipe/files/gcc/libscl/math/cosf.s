	AREA	|C$$code|, CODE, READONLY
	EXPORT	cosf
cosf
	STR	a1, [sp, #-4]!
	LDFS	f0, [sp], #4
	COSS	f0, f0
	MOV	pc, lr

	END