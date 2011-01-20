Index: gcc/config/arm/arm.h
===================================================================
--- gcc/config/arm/arm.h	(revision 168231)
+++ gcc/config/arm/arm.h	(working copy)
@@ -227,7 +227,7 @@
 				         : TARGET_TPCS_FRAME)
 #define TARGET_LDRD			(arm_arch5e && ARM_DOUBLEWORD_ALIGN)
 #define TARGET_AAPCS_BASED \
-    (arm_abi != ARM_ABI_APCS && arm_abi != ARM_ABI_ATPCS)
+    (arm_abi != ARM_ABI_APCS && arm_abi != ARM_ABI_ATPCS && arm_abi != ARM_ABI_APCS32)
 
 #define TARGET_HARD_TP			(target_thread_pointer == TP_CP15)
 #define TARGET_SOFT_TP			(target_thread_pointer == TP_SOFT)
@@ -399,7 +399,8 @@
   ARM_ABI_ATPCS,
   ARM_ABI_AAPCS,
   ARM_ABI_IWMMXT,
-  ARM_ABI_AAPCS_LINUX
+  ARM_ABI_AAPCS_LINUX,
+  ARM_ABI_APCS32
 };
 
 extern enum arm_abi_type arm_abi;
@@ -883,7 +884,11 @@
    : THUMB_HARD_FRAME_POINTER_REGNUM)
 
 #define HARD_FRAME_POINTER_IS_FRAME_POINTER 0
+#ifdef TARGET_RISCOSELF
+#define HARD_FRAME_POINTER_IS_ARG_POINTER 1
+#else
 #define HARD_FRAME_POINTER_IS_ARG_POINTER 0
+#endif
 
 #define FP_REGNUM	                HARD_FRAME_POINTER_REGNUM
 
@@ -906,10 +911,18 @@
   (((REGNUM) >= FIRST_IWMMXT_GR_REGNUM) && ((REGNUM) <= LAST_IWMMXT_GR_REGNUM))
 
 /* Base register for access to local variables of the function.  */
+#ifdef TARGET_RISCOSELF
+#define FRAME_POINTER_REGNUM	9
+#else
 #define FRAME_POINTER_REGNUM	25
+#endif
 
 /* Base register for access to arguments of the function.  */
+#ifdef TARGET_RISCOSELF
+#define ARG_POINTER_REGNUM	ARM_HARD_FRAME_POINTER_REGNUM
+#else
 #define ARG_POINTER_REGNUM	26
+#endif
 
 #define FIRST_CIRRUS_FP_REGNUM	27
 #define LAST_CIRRUS_FP_REGNUM	42
@@ -1387,7 +1400,7 @@
    is at the high-address end of the local variables;
    that is, each additional local variable allocated
    goes at a more negative offset in the frame.  */
-#define FRAME_GROWS_DOWNWARD 1
+#define FRAME_GROWS_DOWNWARD ((arm_abi == ARM_ABI_APCS32) ? 0 : 1)
 
 /* The amount of scratch space needed by _interwork_{r7,r11}_call_via_rN().
    When present, it is one word in size, and sits at the top of the frame,
@@ -1407,7 +1420,8 @@
    If FRAME_GROWS_DOWNWARD, this is the offset to the END of the
    first local allocated.  Otherwise, it is the offset to the BEGINNING
    of the first local allocated.  */
-#define STARTING_FRAME_OFFSET  0
+#define STARTING_FRAME_OFFSET		\
+   (arm_abi == ARM_ABI_APCS32 ? crtl->outgoing_args_size : 0)
 
 /* If we generate an insn to push BYTES bytes,
    this says how many the stack pointer really advances by.  */
@@ -1508,6 +1522,7 @@
   int soft_frame;	/* FRAME_POINTER_REGNUM.  */
   int locals_base;	/* THUMB_HARD_FRAME_POINTER_REGNUM.  */
   int outgoing_args;	/* STACK_POINTER_REGNUM.  */
+  int frame_size;	/* get_frame_size() */
   unsigned int saved_regs_mask;
 }
 arm_stack_offsets;
@@ -1534,6 +1549,8 @@
   /* Records if sibcalls are blocked because an argument
      register is needed to preserve stack alignment.  */
   int sibcall_blocked;
+  /* Non-zero if this is a leaf function.  */
+  int leaf;
   /* The PIC register for this function.  This might be a pseudo.  */
   rtx pic_reg;
   /* Labels for per-function Thumb call-via stubs.  One per potential calling
@@ -1586,6 +1603,8 @@
   int nregs;
   /* This is the number of iWMMXt register arguments scanned so far.  */
   int iwmmxt_nregs;
+  /* This is the number of FPA register arguments scanned so far.  */
+  int fpa_nregs;
   int named_count;
   int nargs;
   /* Which procedure call variant to use for this call.  */
@@ -1636,6 +1655,8 @@
    (IN_RANGE ((REGNO), 0, 3)						\
     || (TARGET_AAPCS_BASED && TARGET_VFP && TARGET_HARD_FLOAT		\
 	&& IN_RANGE ((REGNO), FIRST_VFP_REGNUM, FIRST_VFP_REGNUM + 15))	\
+    || (TARGET_FPA && TARGET_APCS_FLOAT					\
+        && IN_RANGE ((REGNO), FIRST_FPA_REGNUM, FIRST_FPA_REGNUM + 3))	\
     || (TARGET_IWMMXT_ABI						\
 	&& IN_RANGE ((REGNO), FIRST_IWMMXT_REGNUM, FIRST_IWMMXT_REGNUM + 9)))
 
@@ -1722,6 +1743,11 @@
    pointer.  Note we have to use {ARM|THUMB}_HARD_FRAME_POINTER_REGNUM
    because the definition of HARD_FRAME_POINTER_REGNUM is not a constant.  */
 
+#ifdef TARGET_RISCOSELF
+#define ELIMINABLE_REGS                                         \
+{{ ARG_POINTER_REGNUM,        STACK_POINTER_REGNUM            },\
+ { FRAME_POINTER_REGNUM,      STACK_POINTER_REGNUM            }}
+#else
 #define ELIMINABLE_REGS						\
 {{ ARG_POINTER_REGNUM,        STACK_POINTER_REGNUM            },\
  { ARG_POINTER_REGNUM,        FRAME_POINTER_REGNUM            },\
@@ -1730,6 +1756,7 @@
  { FRAME_POINTER_REGNUM,      STACK_POINTER_REGNUM            },\
  { FRAME_POINTER_REGNUM,      ARM_HARD_FRAME_POINTER_REGNUM   },\
  { FRAME_POINTER_REGNUM,      THUMB_HARD_FRAME_POINTER_REGNUM }}
+#endif
 
 /* Define the offset between two registers, one to be eliminated, and the
    other its replacement, at the start of a routine.  */
@@ -1775,8 +1802,8 @@
 /* Don't allow the pc to be used.  */
 #define ARM_REGNO_OK_FOR_BASE_P(REGNO)			\
   (TEST_REGNO (REGNO, <, PC_REGNUM)			\
-   || TEST_REGNO (REGNO, ==, FRAME_POINTER_REGNUM)	\
-   || TEST_REGNO (REGNO, ==, ARG_POINTER_REGNUM))
+   || (!OPTION_APCS_STACK && TEST_REGNO (REGNO, ==, FRAME_POINTER_REGNUM)) \
+   || (!OPTION_APCS_STACK && TEST_REGNO (REGNO, ==, ARG_POINTER_REGNUM)))
 
 #define THUMB1_REGNO_MODE_OK_FOR_BASE_P(REGNO, MODE)		\
   (TEST_REGNO (REGNO, <=, LAST_LO_REGNUM)			\
@@ -1864,6 +1891,11 @@
   if (TARGET_THUMB2)			\
     thumb2_asm_output_opcode (STREAM);
 
+#ifdef TARGET_RISCOSELF
+/* Defining CTORS_SECTION_ASM_OP makes the asm() hacks in crtstuff.c generate
+   code which breaks the module case (cfr. "Hack: force cc1 to switch to .data
+   section early," comment).  */
+#else
 /* The EABI specifies that constructors should go in .init_array.
    Other targets use .ctors for compatibility.  */
 #ifndef ARM_EABI_CTORS_SECTION_OP
@@ -1909,6 +1941,7 @@
 #   define DTORS_SECTION_ASM_OP ARM_DTORS_SECTION_OP
 # endif /* !defined (__ARM_EABI__) */
 #endif /* !defined (IN_LIBCC2) */
+#endif
 
 /* True if the operating system can merge entities with vague linkage
    (e.g., symbols in COMDAT group) during dynamic linking.  */
@@ -1930,8 +1963,8 @@
 #define ARM_REG_OK_FOR_BASE_P(X)		\
   (REGNO (X) <= LAST_ARM_REGNUM			\
    || REGNO (X) >= FIRST_PSEUDO_REGISTER	\
-   || REGNO (X) == FRAME_POINTER_REGNUM		\
-   || REGNO (X) == ARG_POINTER_REGNUM)
+   || (!OPTION_APCS_STACK && REGNO (X) == FRAME_POINTER_REGNUM) \
+   || (!OPTION_APCS_STACK && REGNO (X) == ARG_POINTER_REGNUM))
 
 #define ARM_REG_OK_FOR_INDEX_P(X)		\
   ((REGNO (X) <= LAST_ARM_REGNUM		\