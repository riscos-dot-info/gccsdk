; Tests all support RELOC names.

	AREA Data, DATA, NOINIT

	IMPORT ImpR_ARM_NONE
	DCD 0
	RELOC R_ARM_NONE, ImpR_ARM_NONE
	IMPORT ImpR_ARM_PC24
	DCD 0
	RELOC R_ARM_PC24, ImpR_ARM_PC24
	IMPORT ImpR_ARM_ABS32
	DCD 0
	RELOC R_ARM_ABS32, ImpR_ARM_ABS32
	IMPORT ImpR_ARM_REL32
	DCD 0
	RELOC R_ARM_REL32, ImpR_ARM_REL32
	IMPORT ImpR_ARM_LDR_PC_G0
	DCD 0
	RELOC R_ARM_LDR_PC_G0, ImpR_ARM_LDR_PC_G0
	IMPORT ImpR_ARM_ABS16
	DCD 0
	RELOC R_ARM_ABS16, ImpR_ARM_ABS16
	IMPORT ImpR_ARM_ABS12
	DCD 0
	RELOC R_ARM_ABS12, ImpR_ARM_ABS12
	IMPORT ImpR_ARM_THM_ABS5
	DCD 0
	RELOC R_ARM_THM_ABS5, ImpR_ARM_THM_ABS5
	IMPORT ImpR_ARM_ABS8
	DCD 0
	RELOC R_ARM_ABS8, ImpR_ARM_ABS8
	IMPORT ImpR_ARM_SBREL32
	DCD 0
	RELOC R_ARM_SBREL32, ImpR_ARM_SBREL32
	IMPORT ImpR_ARM_THM_CALL
	DCD 0
	RELOC R_ARM_THM_CALL, ImpR_ARM_THM_CALL
	IMPORT ImpR_ARM_THM_PC8
	DCD 0
	RELOC R_ARM_THM_PC8, ImpR_ARM_THM_PC8
	IMPORT ImpR_ARM_BREL_ADJ
	DCD 0
	RELOC R_ARM_BREL_ADJ, ImpR_ARM_BREL_ADJ
	IMPORT ImpR_ARM_SWI24
	DCD 0
	RELOC R_ARM_SWI24, ImpR_ARM_SWI24
	IMPORT ImpR_ARM_TLS_DESC
	DCD 0
	RELOC R_ARM_TLS_DESC, ImpR_ARM_TLS_DESC
	IMPORT ImpR_ARM_THM_SWI8
	DCD 0
	RELOC R_ARM_THM_SWI8, ImpR_ARM_THM_SWI8
	IMPORT ImpR_ARM_XPC25
	DCD 0
	RELOC R_ARM_XPC25, ImpR_ARM_XPC25
	IMPORT ImpR_ARM_THM_XPC22
	DCD 0
	RELOC R_ARM_THM_XPC22, ImpR_ARM_THM_XPC22
	IMPORT ImpR_ARM_TLS_DTPMOD32
	DCD 0
	RELOC R_ARM_TLS_DTPMOD32, ImpR_ARM_TLS_DTPMOD32
	IMPORT ImpR_ARM_TLS_DTPOFF32
	DCD 0
	RELOC R_ARM_TLS_DTPOFF32, ImpR_ARM_TLS_DTPOFF32
	IMPORT ImpR_ARM_TLS_TPOFF32
	DCD 0
	RELOC R_ARM_TLS_TPOFF32, ImpR_ARM_TLS_TPOFF32
	IMPORT ImpR_ARM_COPY
	DCD 0
	RELOC R_ARM_COPY, ImpR_ARM_COPY
	IMPORT ImpR_ARM_GLOB_DAT
	DCD 0
	RELOC R_ARM_GLOB_DAT, ImpR_ARM_GLOB_DAT
	IMPORT ImpR_ARM_JUMP_SLOT
	DCD 0
	RELOC R_ARM_JUMP_SLOT, ImpR_ARM_JUMP_SLOT
	IMPORT ImpR_ARM_RELATIVE
	DCD 0
	RELOC R_ARM_RELATIVE, ImpR_ARM_RELATIVE
	IMPORT ImpR_ARM_GOTOFF32
	DCD 0
	RELOC R_ARM_GOTOFF32, ImpR_ARM_GOTOFF32
	IMPORT ImpR_ARM_BASE_PREL
	DCD 0
	RELOC R_ARM_BASE_PREL, ImpR_ARM_BASE_PREL
	IMPORT ImpR_ARM_GOT_BREL
	DCD 0
	RELOC R_ARM_GOT_BREL, ImpR_ARM_GOT_BREL
	IMPORT ImpR_ARM_PLT32
	DCD 0
	RELOC R_ARM_PLT32, ImpR_ARM_PLT32
	IMPORT ImpR_ARM_CALL
	DCD 0
	RELOC R_ARM_CALL, ImpR_ARM_CALL
	IMPORT ImpR_ARM_JUMP24
	DCD 0
	RELOC R_ARM_JUMP24, ImpR_ARM_JUMP24
	IMPORT ImpR_ARM_THM_JUMP24
	DCD 0
	RELOC R_ARM_THM_JUMP24, ImpR_ARM_THM_JUMP24
	IMPORT ImpR_ARM_BASE_ABS
	DCD 0
	RELOC R_ARM_BASE_ABS, ImpR_ARM_BASE_ABS
	IMPORT ImpR_ARM_ALU_PCREL_7_0
	DCD 0
	RELOC R_ARM_ALU_PCREL_7_0, ImpR_ARM_ALU_PCREL_7_0
	IMPORT ImpR_ARM_ALU_PCREL_15_8
	DCD 0
	RELOC R_ARM_ALU_PCREL_15_8, ImpR_ARM_ALU_PCREL_15_8
	IMPORT ImpR_ARM_ALU_PCREL_23_15
	DCD 0
	RELOC R_ARM_ALU_PCREL_23_15, ImpR_ARM_ALU_PCREL_23_15
	IMPORT ImpR_ARM_LDR_SBREL_11_0_NC
	DCD 0
	RELOC R_ARM_LDR_SBREL_11_0_NC, ImpR_ARM_LDR_SBREL_11_0_NC
	IMPORT ImpR_ARM_ALU_SBREL_19_12_NC
	DCD 0
	RELOC R_ARM_ALU_SBREL_19_12_NC, ImpR_ARM_ALU_SBREL_19_12_NC
	IMPORT ImpR_ARM_ALU_SBREL_27_20_CK
	DCD 0
	RELOC R_ARM_ALU_SBREL_27_20_CK, ImpR_ARM_ALU_SBREL_27_20_CK
	IMPORT ImpR_ARM_TARGET1
	DCD 0
	RELOC R_ARM_TARGET1, ImpR_ARM_TARGET1
	IMPORT ImpR_ARM_SBREL31
	DCD 0
	RELOC R_ARM_SBREL31, ImpR_ARM_SBREL31
	IMPORT ImpR_ARM_V4BX
	DCD 0
	RELOC R_ARM_V4BX, ImpR_ARM_V4BX
	IMPORT ImpR_ARM_TARGET2
	DCD 0
	RELOC R_ARM_TARGET2, ImpR_ARM_TARGET2
	IMPORT ImpR_ARM_PREL31
	DCD 0
	RELOC R_ARM_PREL31, ImpR_ARM_PREL31
	IMPORT ImpR_ARM_MOVW_ABS_NC
	DCD 0
	RELOC R_ARM_MOVW_ABS_NC, ImpR_ARM_MOVW_ABS_NC
	IMPORT ImpR_ARM_MOVT_ABS
	DCD 0
	RELOC R_ARM_MOVT_ABS, ImpR_ARM_MOVT_ABS
	IMPORT ImpR_ARM_MOVW_PREL_NC
	DCD 0
	RELOC R_ARM_MOVW_PREL_NC, ImpR_ARM_MOVW_PREL_NC
	IMPORT ImpR_ARM_MOVT_PREL
	DCD 0
	RELOC R_ARM_MOVT_PREL, ImpR_ARM_MOVT_PREL
	IMPORT ImpR_ARM_THM_MOVW_ABS_NC
	DCD 0
	RELOC R_ARM_THM_MOVW_ABS_NC, ImpR_ARM_THM_MOVW_ABS_NC
	IMPORT ImpR_ARM_THM_MOVT_ABS
	DCD 0
	RELOC R_ARM_THM_MOVT_ABS, ImpR_ARM_THM_MOVT_ABS
	IMPORT ImpR_ARM_THM_MOVW_PREL_NC
	DCD 0
	RELOC R_ARM_THM_MOVW_PREL_NC, ImpR_ARM_THM_MOVW_PREL_NC
	IMPORT ImpR_ARM_THM_MOVT_PREL
	DCD 0
	RELOC R_ARM_THM_MOVT_PREL, ImpR_ARM_THM_MOVT_PREL
	IMPORT ImpR_ARM_THM_JUMP19
	DCD 0
	RELOC R_ARM_THM_JUMP19, ImpR_ARM_THM_JUMP19
	IMPORT ImpR_ARM_THM_JUMP6
	DCD 0
	RELOC R_ARM_THM_JUMP6, ImpR_ARM_THM_JUMP6
	IMPORT ImpR_ARM_THM_ALU_PREL_11_0
	DCD 0
	RELOC R_ARM_THM_ALU_PREL_11_0, ImpR_ARM_THM_ALU_PREL_11_0
	IMPORT ImpR_ARM_THM_PC12
	DCD 0
	RELOC R_ARM_THM_PC12, ImpR_ARM_THM_PC12
	IMPORT ImpR_ARM_ABS32_NOI
	DCD 0
	RELOC R_ARM_ABS32_NOI, ImpR_ARM_ABS32_NOI
	IMPORT ImpR_ARM_REL32_NOI
	DCD 0
	RELOC R_ARM_REL32_NOI, ImpR_ARM_REL32_NOI
	IMPORT ImpR_ARM_ALU_PC_G0_NC
	DCD 0
	RELOC R_ARM_ALU_PC_G0_NC, ImpR_ARM_ALU_PC_G0_NC
	IMPORT ImpR_ARM_ALU_PC_G0
	DCD 0
	RELOC R_ARM_ALU_PC_G0, ImpR_ARM_ALU_PC_G0
	IMPORT ImpR_ARM_ALU_PC_G1_NC
	DCD 0
	RELOC R_ARM_ALU_PC_G1_NC, ImpR_ARM_ALU_PC_G1_NC
	IMPORT ImpR_ARM_ALU_PC_G1
	DCD 0
	RELOC R_ARM_ALU_PC_G1, ImpR_ARM_ALU_PC_G1
	IMPORT ImpR_ARM_ALU_PC_G2
	DCD 0
	RELOC R_ARM_ALU_PC_G2, ImpR_ARM_ALU_PC_G2
	IMPORT ImpR_ARM_LDR_PC_G1
	DCD 0
	RELOC R_ARM_LDR_PC_G1, ImpR_ARM_LDR_PC_G1
	IMPORT ImpR_ARM_LDR_PC_G2
	DCD 0
	RELOC R_ARM_LDR_PC_G2, ImpR_ARM_LDR_PC_G2
	IMPORT ImpR_ARM_LDRS_PC_G0
	DCD 0
	RELOC R_ARM_LDRS_PC_G0, ImpR_ARM_LDRS_PC_G0
	IMPORT ImpR_ARM_LDRS_PC_G1
	DCD 0
	RELOC R_ARM_LDRS_PC_G1, ImpR_ARM_LDRS_PC_G1
	IMPORT ImpR_ARM_LDRS_PC_G2
	DCD 0
	RELOC R_ARM_LDRS_PC_G2, ImpR_ARM_LDRS_PC_G2
	IMPORT ImpR_ARM_LDC_PC_G0
	DCD 0
	RELOC R_ARM_LDC_PC_G0, ImpR_ARM_LDC_PC_G0
	IMPORT ImpR_ARM_LDC_PC_G1
	DCD 0
	RELOC R_ARM_LDC_PC_G1, ImpR_ARM_LDC_PC_G1
	IMPORT ImpR_ARM_LDC_PC_G2
	DCD 0
	RELOC R_ARM_LDC_PC_G2, ImpR_ARM_LDC_PC_G2
	IMPORT ImpR_ARM_ALU_SB_G0_NC
	DCD 0
	RELOC R_ARM_ALU_SB_G0_NC, ImpR_ARM_ALU_SB_G0_NC
	IMPORT ImpR_ARM_ALU_SB_G0
	DCD 0
	RELOC R_ARM_ALU_SB_G0, ImpR_ARM_ALU_SB_G0
	IMPORT ImpR_ARM_ALU_SB_G1_NC
	DCD 0
	RELOC R_ARM_ALU_SB_G1_NC, ImpR_ARM_ALU_SB_G1_NC
	IMPORT ImpR_ARM_ALU_SB_G1
	DCD 0
	RELOC R_ARM_ALU_SB_G1, ImpR_ARM_ALU_SB_G1
	IMPORT ImpR_ARM_ALU_SB_G2
	DCD 0
	RELOC R_ARM_ALU_SB_G2, ImpR_ARM_ALU_SB_G2
	IMPORT ImpR_ARM_LDR_SB_G0
	DCD 0
	RELOC R_ARM_LDR_SB_G0, ImpR_ARM_LDR_SB_G0
	IMPORT ImpR_ARM_LDR_SB_G1
	DCD 0
	RELOC R_ARM_LDR_SB_G1, ImpR_ARM_LDR_SB_G1
	IMPORT ImpR_ARM_LDR_SB_G2
	DCD 0
	RELOC R_ARM_LDR_SB_G2, ImpR_ARM_LDR_SB_G2
	IMPORT ImpR_ARM_LDRS_SB_G0
	DCD 0
	RELOC R_ARM_LDRS_SB_G0, ImpR_ARM_LDRS_SB_G0
	IMPORT ImpR_ARM_LDRS_SB_G1
	DCD 0
	RELOC R_ARM_LDRS_SB_G1, ImpR_ARM_LDRS_SB_G1
	IMPORT ImpR_ARM_LDRS_SB_G2
	DCD 0
	RELOC R_ARM_LDRS_SB_G2, ImpR_ARM_LDRS_SB_G2
	IMPORT ImpR_ARM_LDC_SB_G0
	DCD 0
	RELOC R_ARM_LDC_SB_G0, ImpR_ARM_LDC_SB_G0
	IMPORT ImpR_ARM_LDC_SB_G1
	DCD 0
	RELOC R_ARM_LDC_SB_G1, ImpR_ARM_LDC_SB_G1
	IMPORT ImpR_ARM_LDC_SB_G2
	DCD 0
	RELOC R_ARM_LDC_SB_G2, ImpR_ARM_LDC_SB_G2
	IMPORT ImpR_ARM_MOVW_BREL_NC
	DCD 0
	RELOC R_ARM_MOVW_BREL_NC, ImpR_ARM_MOVW_BREL_NC
	IMPORT ImpR_ARM_MOVT_BREL
	DCD 0
	RELOC R_ARM_MOVT_BREL, ImpR_ARM_MOVT_BREL
	IMPORT ImpR_ARM_MOVW_BREL
	DCD 0
	RELOC R_ARM_MOVW_BREL, ImpR_ARM_MOVW_BREL
	IMPORT ImpR_ARM_THM_MOVW_BREL_NC
	DCD 0
	RELOC R_ARM_THM_MOVW_BREL_NC, ImpR_ARM_THM_MOVW_BREL_NC
	IMPORT ImpR_ARM_THM_MOVT_BREL
	DCD 0
	RELOC R_ARM_THM_MOVT_BREL, ImpR_ARM_THM_MOVT_BREL
	IMPORT ImpR_ARM_THM_MOVW_BREL
	DCD 0
	RELOC R_ARM_THM_MOVW_BREL, ImpR_ARM_THM_MOVW_BREL
	IMPORT ImpR_ARM_TLS_GOTDESC
	DCD 0
	RELOC R_ARM_TLS_GOTDESC, ImpR_ARM_TLS_GOTDESC
	IMPORT ImpR_ARM_TLS_CALL
	DCD 0
	RELOC R_ARM_TLS_CALL, ImpR_ARM_TLS_CALL
	IMPORT ImpR_ARM_TLS_DESCSEQ
	DCD 0
	RELOC R_ARM_TLS_DESCSEQ, ImpR_ARM_TLS_DESCSEQ
	IMPORT ImpR_ARM_THM_TLS_CALL
	DCD 0
	RELOC R_ARM_THM_TLS_CALL, ImpR_ARM_THM_TLS_CALL
	IMPORT ImpR_ARM_PLT32_ABS
	DCD 0
	RELOC R_ARM_PLT32_ABS, ImpR_ARM_PLT32_ABS
	IMPORT ImpR_ARM_GOT_ABS
	DCD 0
	RELOC R_ARM_GOT_ABS, ImpR_ARM_GOT_ABS
	IMPORT ImpR_ARM_GOT_PREL
	DCD 0
	RELOC R_ARM_GOT_PREL, ImpR_ARM_GOT_PREL
	IMPORT ImpR_ARM_GOT_BREL12
	DCD 0
	RELOC R_ARM_GOT_BREL12, ImpR_ARM_GOT_BREL12
	IMPORT ImpR_ARM_GOTOFF12
	DCD 0
	RELOC R_ARM_GOTOFF12, ImpR_ARM_GOTOFF12
	IMPORT ImpR_ARM_GOTRELAX
	DCD 0
	RELOC R_ARM_GOTRELAX, ImpR_ARM_GOTRELAX
	IMPORT ImpR_ARM_GNU_VTENTRY
	DCD 0
	RELOC R_ARM_GNU_VTENTRY, ImpR_ARM_GNU_VTENTRY
	IMPORT ImpR_ARM_GNU_VTINHERIT
	DCD 0
	RELOC R_ARM_GNU_VTINHERIT, ImpR_ARM_GNU_VTINHERIT
	IMPORT ImpR_ARM_THM_JUMP11
	DCD 0
	RELOC R_ARM_THM_JUMP11, ImpR_ARM_THM_JUMP11
	IMPORT ImpR_ARM_THM_JUMP8
	DCD 0
	RELOC R_ARM_THM_JUMP8, ImpR_ARM_THM_JUMP8
	IMPORT ImpR_ARM_TLS_GD32
	DCD 0
	RELOC R_ARM_TLS_GD32, ImpR_ARM_TLS_GD32
	IMPORT ImpR_ARM_TLS_LDM32
	DCD 0
	RELOC R_ARM_TLS_LDM32, ImpR_ARM_TLS_LDM32
	IMPORT ImpR_ARM_TLS_LDO32
	DCD 0
	RELOC R_ARM_TLS_LDO32, ImpR_ARM_TLS_LDO32
	IMPORT ImpR_ARM_TLS_IE32
	DCD 0
	RELOC R_ARM_TLS_IE32, ImpR_ARM_TLS_IE32
	IMPORT ImpR_ARM_TLS_LE32
	DCD 0
	RELOC R_ARM_TLS_LE32, ImpR_ARM_TLS_LE32
	IMPORT ImpR_ARM_TLS_LDO12
	DCD 0
	RELOC R_ARM_TLS_LDO12, ImpR_ARM_TLS_LDO12
	IMPORT ImpR_ARM_TLS_LE12
	DCD 0
	RELOC R_ARM_TLS_LE12, ImpR_ARM_TLS_LE12
	IMPORT ImpR_ARM_TLS_IE12GP
	DCD 0
	RELOC R_ARM_TLS_IE12GP, ImpR_ARM_TLS_IE12GP
	IMPORT ImpR_ARM_PRIVATE_0
	DCD 0
	RELOC R_ARM_PRIVATE_0, ImpR_ARM_PRIVATE_0
	IMPORT ImpR_ARM_PRIVATE_1
	DCD 0
	RELOC R_ARM_PRIVATE_1, ImpR_ARM_PRIVATE_1
	IMPORT ImpR_ARM_PRIVATE_2
	DCD 0
	RELOC R_ARM_PRIVATE_2, ImpR_ARM_PRIVATE_2
	IMPORT ImpR_ARM_PRIVATE_3
	DCD 0
	RELOC R_ARM_PRIVATE_3, ImpR_ARM_PRIVATE_3
	IMPORT ImpR_ARM_PRIVATE_4
	DCD 0
	RELOC R_ARM_PRIVATE_4, ImpR_ARM_PRIVATE_4
	IMPORT ImpR_ARM_PRIVATE_5
	DCD 0
	RELOC R_ARM_PRIVATE_5, ImpR_ARM_PRIVATE_5
	IMPORT ImpR_ARM_PRIVATE_6
	DCD 0
	RELOC R_ARM_PRIVATE_6, ImpR_ARM_PRIVATE_6
	IMPORT ImpR_ARM_PRIVATE_7
	DCD 0
	RELOC R_ARM_PRIVATE_7, ImpR_ARM_PRIVATE_7
	IMPORT ImpR_ARM_PRIVATE_8
	DCD 0
	RELOC R_ARM_PRIVATE_8, ImpR_ARM_PRIVATE_8
	IMPORT ImpR_ARM_PRIVATE_9
	DCD 0
	RELOC R_ARM_PRIVATE_9, ImpR_ARM_PRIVATE_9
	IMPORT ImpR_ARM_PRIVATE_10
	DCD 0
	RELOC R_ARM_PRIVATE_10, ImpR_ARM_PRIVATE_10
	IMPORT ImpR_ARM_PRIVATE_11
	DCD 0
	RELOC R_ARM_PRIVATE_11, ImpR_ARM_PRIVATE_11
	IMPORT ImpR_ARM_PRIVATE_12
	DCD 0
	RELOC R_ARM_PRIVATE_12, ImpR_ARM_PRIVATE_12
	IMPORT ImpR_ARM_PRIVATE_13
	DCD 0
	RELOC R_ARM_PRIVATE_13, ImpR_ARM_PRIVATE_13
	IMPORT ImpR_ARM_PRIVATE_14
	DCD 0
	RELOC R_ARM_PRIVATE_14, ImpR_ARM_PRIVATE_14
	IMPORT ImpR_ARM_PRIVATE_15
	DCD 0
	RELOC R_ARM_PRIVATE_15, ImpR_ARM_PRIVATE_15
	IMPORT ImpR_ARM_ME_TOO
	DCD 0
	RELOC R_ARM_ME_TOO, ImpR_ARM_ME_TOO
	IMPORT ImpR_ARM_THM_TLS_DESCSEQ16
	DCD 0
	RELOC R_ARM_THM_TLS_DESCSEQ16, ImpR_ARM_THM_TLS_DESCSEQ16
	IMPORT ImpR_ARM_THM_TLS_DESCSEQ32
	DCD 0
	RELOC R_ARM_THM_TLS_DESCSEQ32, ImpR_ARM_THM_TLS_DESCSEQ32
	IMPORT ImpR_ARM_THM_GOT_BREL12
	DCD 0
	RELOC R_ARM_THM_GOT_BREL12, ImpR_ARM_THM_GOT_BREL12
	IMPORT ImpR_ARM_IRELATIVE
	DCD 0
	RELOC R_ARM_IRELATIVE, ImpR_ARM_IRELATIVE

	END
