Index: gcc/config/arm/arm-protos.h
===================================================================
--- gcc/config/arm/arm-protos.h	(revision 179360)
+++ gcc/config/arm/arm-protos.h	(working copy)
@@ -244,4 +244,13 @@
 extern const struct tune_params *current_tune;
 #endif /* RTX_CODE */
 
+extern bool arm_apcs_frame_needed (void);
+extern void arm_expand_save_stack_block (rtx, rtx);
+extern void arm_expand_restore_stack_block (rtx, rtx);
+extern void arm_expand_alloca_epilogue (void);
+extern void arm_expand_allocate_stack (rtx, rtx);
+extern void arm_expand_save_stack_nonlocal (rtx *);
+extern void arm_expand_restore_stack_nonlocal (rtx *);
+extern void arm_expand_nonlocal_goto (rtx *);
+
 #endif /* ! GCC_ARM_PROTOS_H */