	AREA Data, DATA, READONLY

	[ :LNOT:REFERENCE

	DCD 5*3
	DCD -5*3
	DCD 3*-5
	DCD 11/2
	DCD -11/2	; Unsigned division
	DCD 11/-2	; Unsigned division
	DCD 11/3*2
	DCD 12/3/3

	DCD 11:MOD:2
	DCD -11:MOD:2
	DCD 11:MOD:-2
	DCD -3:MOD:-2

	|

	DCD 15
	DCD -15
	DCD -15
	DCD 5
	DCD &7FFFFFFA
	DCD 0
	DCD 6
	DCD 1

	DCD 1
	DCD 1
	DCD 11
	DCD -3

	]

	END