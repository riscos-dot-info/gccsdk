; Tests CPS

	AREA	Code, CODE

	[ :LNOT: REFERENCE

	CPS	# 16
	CPS	# 0x1F

	CPSIE	, #16
	CPSID	, #16

	CPSIE	iF
	CPSIE	a, #18

	|

	DCI	&f1020010
	DCI	&f102001f
	DCI	&f10a0010
	DCI	&f10e0010
	DCI	&f10800c0
	DCI	&f10a0112

	]

	END
