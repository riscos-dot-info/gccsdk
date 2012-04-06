--- mono/mini/mini-arm.h.orig	2011-08-26 02:06:56.000000000 +0100
+++ mono/mini/mini-arm.h	2012-01-15 20:16:14.000000000 +0000
@@ -131,6 +131,9 @@
 	gpointer seq_point_bp_method_var;
 	gboolean omit_fp, omit_fp_computed;
 	gpointer cinfo;
+#ifdef ENABLE_RISCOS_STACKFRAMES
+	gboolean stack_frame_needed;
+#endif
 } MonoCompileArch;
 
 #define MONO_ARCH_EMULATE_FCONV_TO_I8 1
@@ -180,7 +183,12 @@
 #endif
 
 /* ARM doesn't have too many registers, so we have to use a callee saved one */
+#ifdef ENABLE_RISCOS_STACKFRAMES
+/* V5 is corrupted by PLTs on RISC OS.  */
+#define MONO_ARCH_RGCTX_REG ARMREG_V4
+#else
 #define MONO_ARCH_RGCTX_REG ARMREG_V5
+#endif
 /* First argument reg */
 #define MONO_ARCH_VTABLE_REG ARMREG_R0
 
@@ -221,5 +229,10 @@
 guint8*
 mono_arm_get_thumb_plt_entry (guint8 *code) MONO_INTERNAL;
 
+#ifdef ENABLE_RISCOS_STACKFRAMES
+extern void (*__rt_stkovf_split_small)(void);
+extern void (*__rt_stkovf_split_big)(void);
+#endif
+
 #endif /* __MONO_MINI_ARM_H__ */
 