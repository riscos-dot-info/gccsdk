		AREA |C$$code|, CODE, READONLY

		; Multiply/Multiply Accumulate

	[ :LNOT: REFERENCE
		SMLAL	R0,R1,R2,R3
		SMLALS	R0,R1,R2,R3

		SMULL	R0,R1,R2,R3
		SMULLS	R0,R1,R2,R3

		UMLAL	R0,R1,R2,R3
		UMLALS	R0,R1,R2,R3

		UMULL	R0,R1,R2,R3
		UMULLS	R0,R1,R2,R3
	|
		DCD	&E0E10392
		DCD	&E0F10392

		DCD	&E0C10392
		DCD	&E0D10392

		DCD	&E0A10392
		DCD	&E0B10392

		DCD	&E0810392
		DCD	&E0910392
	]

		END