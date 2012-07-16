; Tests option -CPU=ARM710T
; RUNOPT: -CPU=ARM710T

	GET include/output_builtin_variables.s

	[ REFERENCE
	= "{CPU} : ARM710T\n"
	= "{ARCHITECTURE} : 4T\n"
	= "{FPU} : FPE2\n"

	= "{TARGET_ARCH_ARM} : 00000004\n"
	= "{TARGET_ARCH_THUMB} : 00000001\n"

	= "{TARGET_ARCH_1} : F\n"
	= "{TARGET_ARCH_2} : F\n"
	= "{TARGET_ARCH_2A} : F\n"
	= "{TARGET_ARCH_3} : F\n"
	= "{TARGET_ARCH_3G} : F\n"
	= "{TARGET_ARCH_3M} : F\n"
	= "{TARGET_ARCH_4XM} : F\n"
	= "{TARGET_ARCH_4} : F\n"
	= "{TARGET_ARCH_4TXM} : F\n"
	= "{TARGET_ARCH_4T} : T\n"
	= "{TARGET_ARCH_5XM} : F\n"
	= "{TARGET_ARCH_5} : F\n"
	= "{TARGET_ARCH_5TXM} : F\n"
	= "{TARGET_ARCH_5T} : F\n"
	= "{TARGET_ARCH_5TEXP} : F\n"
	= "{TARGET_ARCH_5TE} : F\n"
	= "{TARGET_ARCH_5TEJ} : F\n"
	= "{TARGET_ARCH_6} : F\n"
	= "{TARGET_ARCH_6K} : F\n"
	= "{TARGET_ARCH_6T2} : F\n"
	= "{TARGET_ARCH_6Z} : F\n"
	= "{TARGET_ARCH_6_M} : F\n"
	= "{TARGET_ARCH_6S_M} : F\n"
	= "{TARGET_ARCH_7} : F\n"
	= "{TARGET_ARCH_7_A} : F\n"
	= "{TARGET_ARCH_7_R} : F\n"
	= "{TARGET_ARCH_7_M} : F\n"
	= "{TARGET_ARCH_7E_M} : F\n"

	= "{TARGET_FEATURE_CLZ} : F\n"
	= "{TARGET_FEATURE_DIVIDE} : F\n"
	= "{TARGET_FEATURE_DOUBLEWORD} : F\n"
	= "{TARGET_FEATURE_DSPMUL} : F\n"
	= "{TARGET_FEATURE_EXTENSION_REGISTER_COUNT} : 00000000\n"
	= "{TARGET_FEATURE_MULTIPLY} : T\n"
	= "{TARGET_FEATURE_MULTIPROCESSING} : F\n"
	= "{TARGET_FEATURE_NEON} : F\n"
	= "{TARGET_FEATURE_NEON_FP16} : F\n"
	= "{TARGET_FEATURE_NEON_FP32} : F\n"
	= "{TARGET_FEATURE_NEON_INTEGER} : F\n"
	= "{TARGET_FEATURE_UNALIGNED} : F\n"

	= "{TARGET_FPU_FPA} : T\n"
	= "{TARGET_FPU_SOFTFPA} : F\n"
	= "{TARGET_FPU_SOFTFPA_FPA} : F\n"
	= "{TARGET_FPU_SOFTFPA_VFP} : F\n"
	= "{TARGET_FPU_SOFTVFP} : F\n"
	= "{TARGET_FPU_SOFTVFP_FPA} : F\n"
	= "{TARGET_FPU_SOFTVFP_VFP} : F\n"
	= "{TARGET_FPU_VFP} : F\n"
	= "{TARGET_FPU_VFPV1} : F\n"
	= "{TARGET_FPU_VFPV2} : F\n"
	= "{TARGET_FPU_VFPV3} : F\n"
	= "{TARGET_FPU_VFPV4} : F\n"

	= "{TARGET_PROFILE_A} : F\n"
	= "{TARGET_PROFILE_M} : F\n"
	= "{TARGET_PROFILE_R} : F\n"
	]

	END
