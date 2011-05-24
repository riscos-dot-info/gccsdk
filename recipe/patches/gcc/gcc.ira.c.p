Index: gcc/ira.c
===================================================================
--- gcc/ira.c	(revision 174131)
+++ gcc/ira.c	(working copy)
@@ -1775,7 +1775,9 @@
      sp-adjusting insns for this case.  */
   int need_fp
     = (! flag_omit_frame_pointer
+#ifndef TARGET_RISCOSELF
        || (cfun->calls_alloca && EXIT_IGNORE_STACK)
+#endif
        /* We need the frame pointer to catch stack overflow exceptions
 	  if the stack pointer is moving.  */
        || (flag_stack_check && STACK_CHECK_MOVING_SP)
