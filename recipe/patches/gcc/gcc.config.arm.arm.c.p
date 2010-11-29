Index: gcc/config/arm/arm.c
===================================================================
--- gcc/config/arm/arm.c	(revision 167269)
+++ gcc/config/arm/arm.c	(working copy)
@@ -124,6 +124,8 @@
 #if TARGET_DLLIMPORT_DECL_ATTRIBUTES
 static tree arm_handle_notshared_attribute (tree *, tree, tree, int, bool *);
 #endif
+static int arm_leaf_function_p (void);
+static void require_pic_register (void);
 static void arm_output_function_epilogue (FILE *, HOST_WIDE_INT);
 static void arm_output_function_prologue (FILE *, HOST_WIDE_INT);
 static void thumb1_output_function_prologue (FILE *, HOST_WIDE_INT);
@@ -1017,7 +1019,8 @@
   {"atpcs",   ARM_ABI_ATPCS},
   {"aapcs",   ARM_ABI_AAPCS},
   {"iwmmxt",  ARM_ABI_IWMMXT},
-  {"aapcs-linux",   ARM_ABI_AAPCS_LINUX}
+  {"aapcs-linux",   ARM_ABI_AAPCS_LINUX},
+  {"apcs-32",  ARM_ABI_APCS32}
 };
 
 /* Supported TLS relocations.  */
@@ -1319,6 +1322,8 @@
 
 /* Implement TARGET_HANDLE_OPTION.  */
 
+static int target_apcs_stack_check_explicit; /* HACK */
+
 static bool
 arm_handle_option (size_t code, const char *arg, int value ATTRIBUTE_UNUSED)
 {
@@ -1344,6 +1349,10 @@
       arm_selected_tune = arm_find_cpu(arg, all_cores, "-mtune");
       return true;
 
+    case OPT_mapcs_stack_check:
+      target_apcs_stack_check_explicit = 1;
+      return true;
+
     default:
       return true;
     }
@@ -1637,12 +1646,38 @@
   if (TARGET_ARM && TARGET_CALLEE_INTERWORKING)
     warning (0, "enabling callee interworking support is only meaningful when compiling for the Thumb");
 
-  if (TARGET_APCS_STACK && !TARGET_APCS_FRAME)
+  if (OPTION_APCS_STACK && !TARGET_APCS_FRAME)
     {
       warning (0, "-mapcs-stack-check incompatible with -mno-apcs-frame");
       target_flags |= MASK_APCS_FRAME;
     }
 
+#ifdef TARGET_RISCOSELF
+  /* FIXME: we better would have a test arm_abi == ARM_ABI_APCS32 instead but
+     arm_abi is only defined further on.  */
+  /* Unless the option -m(no-)apcs-stack-check is explicitely set, we have it
+     default set for APCS-32 code unless we're generating module code
+     (-mmodule) as this is generally using a non-chunk stack anyway.  */
+  if (!target_apcs_stack_check_explicit && !TARGET_MODULE)
+    target_apcs_stack_check |= OPTION_MASK_APCS_STACK;
+
+  /* For RISC OS module support, we need -fpic specified.  */
+  if (TARGET_MODULE)
+    {
+      if (flag_pic == 2)
+	error ("-mmodule and -fPIC are incompatible");
+      flag_pic = 1;
+    }
+#endif
+
+#if !defined(ARM_STKOVF_SPLIT_SMALL) || !defined(ARM_STKOVF_SPLIT_BIG)
+  if (OPTION_APCS_STACK)
+    {
+      warning (0, "APCS stack checking not supported.  Ignored");
+      target_apcs_stack_check &= ~OPTION_MASK_APCS_STACK;
+    }
+#endif
+
   if (TARGET_POKE_FUNCTION_NAME)
     target_flags |= MASK_APCS_FRAME;
 
@@ -1660,8 +1695,10 @@
       && (TARGET_DEFAULT & MASK_APCS_FRAME))
     warning (0, "-g with -mno-apcs-frame may not give sensible debugging");
 
+#ifndef TARGET_RISCOSELF
   if (TARGET_APCS_FLOAT)
     warning (0, "passing floating point arguments in fp regs not yet supported");
+#endif
 
   /* Initialize boolean versions of the flags, for use in the arm.md file.  */
   arm_arch3m = (insn_flags & FL_ARCH3M) != 0;
@@ -1837,6 +1874,20 @@
   if (TARGET_SOFT_FLOAT)
     arm_fpu_attr = FPU_NONE;
 
+#ifdef TARGET_RISCOSELF
+  if (arm_float_abi == ARM_FLOAT_ABI_SOFTFP)
+    sorry ("arm-unknown-riscos and -mfloat-abi=softfp");
+
+  if (TARGET_SOFT_FLOAT && !TARGET_UNIXLIB)
+    sorry ("-mlibscl and -mfloat-abi=soft");
+
+  if (TARGET_UNIXLIB && TARGET_MODULE)
+    sorry ("-munixlib and -mmodule");
+
+  if (TARGET_THUMB)
+    sorry ("arm-unknown-riscos and thumb");
+#endif
+
   if (TARGET_AAPCS_BASED)
     {
       if (arm_abi == ARM_ABI_IWMMXT)
@@ -1912,17 +1963,23 @@
       flag_pic = 0;
     }
 
+#ifdef TARGET_RISCOSELF
+  /* RISC OS Loader always expects r7 to be used.  For module code this is r8.  */
+  if (flag_pic)
+    arm_pic_register = TARGET_MODULE ? 8 : 7;
+#else
   /* If stack checking is disabled, we can use r10 as the PIC register,
      which keeps r9 available.  The EABI specifies r9 as the PIC register.  */
   if (flag_pic && TARGET_SINGLE_PIC_BASE)
     {
       if (TARGET_VXWORKS_RTP)
 	warning (0, "RTP PIC is incompatible with -msingle-pic-base");
-      arm_pic_register = (TARGET_APCS_STACK || TARGET_AAPCS_BASED) ? 9 : 10;
+      arm_pic_register = (OPTION_APCS_STACK || TARGET_AAPCS_BASED) ? 9 : 10;
     }
 
   if (flag_pic && TARGET_VXWORKS_RTP)
     arm_pic_register = 9;
+#endif
 
   if (arm_pic_register_string != NULL)
     {
@@ -2258,11 +2315,14 @@
 
   /* So do interrupt functions that use the frame pointer and Thumb
      interrupt functions.  */
-  if (IS_INTERRUPT (func_type) && (frame_pointer_needed || TARGET_THUMB))
+  if (IS_INTERRUPT (func_type) && (arm_apcs_frame_needed () || TARGET_THUMB))
     return 0;
 
   offsets = arm_get_frame_offsets ();
-  stack_adjust = offsets->outgoing_args - offsets->saved_regs;
+  if (arm_abi == ARM_ABI_APCS32)
+    stack_adjust = abs (- offsets->frame_size - offsets->outgoing_args);
+  else
+    stack_adjust = offsets->outgoing_args - offsets->saved_regs;
 
   /* As do variadic functions.  */
   if (crtl->args.pretend_args_size
@@ -2270,10 +2330,10 @@
       /* Or if the function calls __builtin_eh_return () */
       || crtl->calls_eh_return
       /* Or if the function calls alloca */
-      || cfun->calls_alloca
+      || (cfun->calls_alloca && ! TARGET_APCS_FRAME)
       /* Or if there is a stack adjustment.  However, if the stack pointer
 	 is saved on the stack, we can use a pre-incrementing stack load.  */
-      || !(stack_adjust == 0 || (TARGET_APCS_FRAME && frame_pointer_needed
+      || !(stack_adjust == 0 || (TARGET_APCS_FRAME && arm_apcs_frame_needed ()
 				 && stack_adjust == 4)))
     return 0;
 
@@ -4531,6 +4591,7 @@
       pcum->aapcs_partial = 0;
       pcum->aapcs_arg_processed = false;
       pcum->aapcs_cprc_slot = -1;
+      pcum->fpa_nregs = 0;
       pcum->can_split = true;
 
       if (pcum->pcs_variant != ARM_PCS_AAPCS)
@@ -4551,6 +4612,7 @@
   /* On the ARM, the offset starts at 0.  */
   pcum->nregs = 0;
   pcum->iwmmxt_nregs = 0;
+  pcum->fpa_nregs = 0;
   pcum->can_split = true;
 
   /* Varargs vectors are treated the same as long long.
@@ -4633,6 +4695,16 @@
 	}
     }
 
+  if (TARGET_FPA && TARGET_APCS_FLOAT
+      && (mode == SFmode || mode == DFmode))
+    {
+      if (pcum->fpa_nregs <= 3)
+	return gen_rtx_REG (mode, pcum->fpa_nregs + FIRST_FPA_REGNUM);
+
+      pcum->can_split = false;
+      return NULL_RTX;
+    }
+
   /* Put doubleword aligned quantities in even register pairs.  */
   if (pcum->nregs & 1
       && ARM_DOUBLEWORD_ALIGN
@@ -4676,6 +4748,10 @@
   if (TARGET_IWMMXT_ABI && arm_vector_mode_supported_p (mode))
     return 0;
 
+  else if (TARGET_FPA && TARGET_APCS_FLOAT
+	   && (mode == DFmode || mode == SFmode))
+    return 0;
+
   if (NUM_ARG_REGS > nregs
       && (NUM_ARG_REGS < nregs + ARM_NUM_REGS2 (mode, type))
       && pcum->can_split)
@@ -4716,6 +4792,9 @@
 	  && pcum->named_count > pcum->nargs
 	  && TARGET_IWMMXT_ABI)
 	pcum->iwmmxt_nregs += 1;
+      else if (TARGET_FPA && TARGET_APCS_FLOAT
+	       && (mode == SFmode || mode == DFmode))
+	pcum->fpa_nregs += 1;
       else
 	pcum->nregs += ARM_NUM_REGS2 (mode, type);
     }
@@ -5064,6 +5143,25 @@
   if (IS_STACKALIGN (func_type))
     return false;
 
+  /* When stack checking we can only allow a sibcall when there is
+     exactly one function call, and that is the candidate for the
+     sibcall.  In other words, we can prevent a full stack-frame
+     being setup.  */
+  if (OPTION_APCS_STACK
+      && (arm_apcs_frame_needed () || crtl->outgoing_args_size))
+    return false;
+
+#ifdef TARGET_RISCOSELF
+  /* When compiling PIC don't tail-call an external function. The PLT
+     requires that the PIC register be loaded by the caller, but the caller
+     must restore the PIC register before the call.  */
+  if (flag_pic == 2 && TREE_PUBLIC (decl))
+    {
+      require_pic_register ();
+      return false;
+    }
+#endif
+
   /* Everything else is ok.  */
   return true;
 }
@@ -5153,6 +5251,49 @@
 	  reg = gen_reg_rtx (Pmode);
 	}
 
+#ifdef TARGET_RISCOSELF
+      if (TARGET_MODULE)
+	{
+	  tree ftree;
+
+	  insn = emit_insn (gen_pic_load_addr_32bit (reg, orig));
+
+	  /* Don't do reallocation based on PIC register when our
+	     symbol is a function or read-only data, i.e. do this only for
+	     global/local writable variables.  */
+	  /* FIXME: this requires more attention : in case ftree is NULL
+	     it is not clear what should be done with symbols generated
+	     by the compiler itself and not coming from src code.  */
+	  ftree = SYMBOL_REF_DECL (orig);
+#if 0
+	  printf ("legitimize_pic_address: sym=%s, ftree=%p\n",
+		  XSTR (orig, 0), (void *)ftree);
+	  if (ftree != NULL)
+	    printf ("  tree code=%d (func %d, var %d), RO data=%d, const data=%d %d\n",
+		    TREE_CODE (ftree), FUNCTION_DECL, VAR_DECL,
+		    decl_readonly_section (ftree, 1),
+		    TREE_READONLY (ftree),
+		    TREE_CONSTANT (ftree));
+#endif
+	  gcc_assert (ftree == NULL || TREE_CODE (ftree) == FUNCTION_DECL || TREE_CODE (ftree) == VAR_DECL);
+	  if (ftree != NULL
+	      && TREE_CODE (ftree) == VAR_DECL
+	      && ! decl_readonly_section (ftree, 1)
+	      && ! (TREE_READONLY (ftree) || TREE_CONSTANT (ftree)))
+	    {
+#if 0
+	      printf("  -> applied OFFSET\n");
+#endif
+
+	      /* If this function doesn't have a pic register, create one now.  */
+	      require_pic_register ();
+
+	      insn = emit_insn (gen_addsi3 (reg, reg, pic_offset_table_rtx));
+	    }
+	}
+      else
+	{
+#endif
       /* VxWorks does not impose a fixed gap between segments; the run-time
 	 gap can be different from the object-file gap.  We therefore can't
 	 use GOTOFF unless we are absolutely sure that the symbol is in the
@@ -5183,6 +5324,7 @@
 
 	  insn = emit_insn (pat);
 	}
+	}
 
       /* Put a REG_EQUAL note on this insn, so that it can be optimized
 	 by loop.  */
@@ -5332,6 +5474,17 @@
   gcc_assert (flag_pic);
 
   pic_reg = cfun->machine->pic_reg;
+  if (TARGET_MODULE)
+    {
+      rtx sl_reg, sb_ref;
+
+      sl_reg = gen_rtx_REG (Pmode, SL_REGNUM);
+      sb_ref = gen_rtx_MEM (Pmode,
+			    gen_rtx_PLUS (Pmode, sl_reg, GEN_INT (-536)));
+      MEM_READONLY_P (sb_ref) = 1;
+      emit_insn (gen_rtx_SET (SImode, pic_reg, sb_ref));
+    }
+  else
   if (TARGET_VXWORKS_RTP)
     {
       pic_rtx = gen_rtx_SYMBOL_REF (Pmode, VXWORKS_GOTT_BASE);
@@ -5354,7 +5507,7 @@
 
       /* On the ARM the PC register contains 'dot + 8' at the time of the
 	 addition, on the Thumb it is 'dot + 4'.  */
-      pic_rtx = plus_constant (l1, TARGET_ARM ? 8 : 4);
+      pic_rtx = plus_constant (l1, TARGET_ARM && flag_pic != 2 ? 8 : 4);
       pic_rtx = gen_rtx_UNSPEC (Pmode, gen_rtvec (1, pic_rtx),
 				UNSPEC_GOTSYM_OFF);
       pic_rtx = gen_rtx_CONST (Pmode, pic_rtx);
@@ -5363,7 +5516,15 @@
 	{
 	  emit_insn (gen_pic_load_addr_32bit (pic_reg, pic_rtx));
 	  if (TARGET_ARM)
-	    emit_insn (gen_pic_add_dot_plus_eight (pic_reg, pic_reg, labelno));
+	    {
+	      emit_insn (gen_pic_add_dot_plus_eight (pic_reg, pic_reg, labelno));
+	      if (flag_pic == 2) /* -fPIC */
+		{
+		  emit_insn (gen_rt_loadpic (pic_reg,
+					     gen_rtx_REG (SImode, PIC_PLT_SCRATCH_REGNUM)));
+		  emit_insn (gen_blockage ());
+		}
+	    }
 	  else
 	    emit_insn (gen_pic_add_dot_plus_four (pic_reg, pic_reg, labelno));
 	}
@@ -13788,7 +13949,7 @@
 	 compromise save just the frame pointers.  Combined with the link
 	 register saved elsewhere this should be sufficient to get
 	 a backtrace.  */
-      if (frame_pointer_needed)
+      if (arm_apcs_frame_needed ())
 	save_reg_mask |= 1 << HARD_FRAME_POINTER_REGNUM;
       if (df_regs_ever_live_p (ARM_HARD_FRAME_POINTER_REGNUM))
 	save_reg_mask |= 1 << ARM_HARD_FRAME_POINTER_REGNUM;
@@ -13804,7 +13965,7 @@
 	  save_reg_mask |= (1 << reg);
 
       /* Handle the frame pointer as a special case.  */
-      if (frame_pointer_needed)
+      if (arm_apcs_frame_needed ())
 	save_reg_mask |= 1 << HARD_FRAME_POINTER_REGNUM;
 
       /* If we aren't loading the PIC register,
@@ -13813,7 +13974,8 @@
 	  && !TARGET_SINGLE_PIC_BASE
 	  && arm_pic_register != INVALID_REGNUM
 	  && (df_regs_ever_live_p (PIC_OFFSET_TABLE_REGNUM)
-	      || crtl->uses_pic_offset_table))
+	      || crtl->uses_pic_offset_table
+	      || !arm_leaf_function_p ()))
 	save_reg_mask |= 1 << PIC_OFFSET_TABLE_REGNUM;
 
       /* The prologue will copy SP into R0, so save it.  */
@@ -13848,7 +14010,7 @@
   unsigned long func_type = arm_current_func_type ();
   int static_chain_stack_bytes = 0;
 
-  if (TARGET_APCS_FRAME && frame_pointer_needed && TARGET_ARM &&
+  if (TARGET_APCS_FRAME && arm_apcs_frame_needed () && TARGET_ARM &&
       IS_NESTED (func_type) &&
       df_regs_ever_live_p (3) && crtl->args.pretend_args_size == 0)
     static_chain_stack_bytes = 4;
@@ -13874,7 +14036,7 @@
 
   /* If we are creating a stack frame, then we must save the frame pointer,
      IP (which will hold the old stack pointer), LR and the PC.  */
-  if (TARGET_APCS_FRAME && frame_pointer_needed && TARGET_ARM)
+  if (TARGET_APCS_FRAME && arm_apcs_frame_needed () && TARGET_ARM)
     save_reg_mask |=
       (1 << ARM_HARD_FRAME_POINTER_REGNUM)
       | (1 << IP_REGNUM)
@@ -13964,7 +14126,7 @@
     mask |= 1 << PIC_OFFSET_TABLE_REGNUM;
 
   /* See if we might need r11 for calls to _interwork_r11_call_via_rN().  */
-  if (!frame_pointer_needed && CALLER_INTERWORKING_SLOT_SIZE > 0)
+  if (!arm_apcs_frame_needed () && CALLER_INTERWORKING_SLOT_SIZE > 0)
     mask |= 1 << ARM_HARD_FRAME_POINTER_REGNUM;
 
   /* LR will also be pushed if any lo regs are pushed.  */
@@ -14125,7 +14287,7 @@
 	     corrupted it, or 3) it was saved to align the stack on
 	     iWMMXt.  In case 1, restore IP into SP, otherwise just
 	     restore IP.  */
-	  if (frame_pointer_needed)
+	  if (arm_apcs_frame_needed ())
 	    {
 	      live_regs_mask &= ~ (1 << IP_REGNUM);
 	      live_regs_mask |=   (1 << SP_REGNUM);
@@ -14158,8 +14320,18 @@
 
 	  /* Generate the load multiple instruction to restore the
 	     registers.  Note we can get here, even if
-	     frame_pointer_needed is true, but only if sp already
+	     arm_apcs_frame_needed () is true, but only if sp already
 	     points to the base of the saved core registers.  */
+	  if (arm_abi == ARM_ABI_APCS32)
+	    {
+	      if (arm_apcs_frame_needed ())
+		sprintf (instr, "ldm%sea\t%%|fp, {", conditional);
+	      else if (live_regs_mask & (1 << SP_REGNUM))
+		sprintf (instr, "ldm%sfd\t%%|sp, {", conditional);
+	      else
+		sprintf (instr, "ldm%sfd\t%%|sp!, {", conditional);
+	    }
+	  else
 	  if (live_regs_mask & (1 << SP_REGNUM))
 	    {
 	      unsigned HOST_WIDE_INT stack_adjust;
@@ -14360,12 +14532,13 @@
   if (IS_STACKALIGN (func_type))
     asm_fprintf (f, "\t%@ Stack Align: May be called with mis-aligned SP.\n");
 
-  asm_fprintf (f, "\t%@ args = %d, pretend = %d, frame = %wd\n",
+  asm_fprintf (f, "\t%@ args = %d, pretend = %d, frame = %wd, outgoing = %d\n",
 	       crtl->args.size,
-	       crtl->args.pretend_args_size, frame_size);
+	       crtl->args.pretend_args_size, frame_size,
+	       crtl->outgoing_args_size);
 
   asm_fprintf (f, "\t%@ frame_needed = %d, uses_anonymous_args = %d\n",
-	       frame_pointer_needed,
+	       arm_apcs_frame_needed (),
 	       cfun->machine->uses_anonymous_args);
 
   if (cfun->machine->lr_save_eliminated)
@@ -14432,7 +14605,7 @@
     if (saved_regs_mask & (1 << reg))
       floats_offset += 4;
 
-  if (TARGET_APCS_FRAME && frame_pointer_needed && TARGET_ARM)
+  if (TARGET_APCS_FRAME && arm_apcs_frame_needed () && TARGET_ARM)
     {
       /* This variable is for the Virtual Frame Pointer, not VFP regs.  */
       int vfp_offset = offsets->frame;
@@ -14556,6 +14729,10 @@
       else
 	saved_regs_mask &= ~ (1 << PC_REGNUM);
 
+      if (arm_abi == ARM_ABI_APCS32)
+	print_multi_reg (f, "ldmea\t%r, ", FP_REGNUM, saved_regs_mask, 0);
+      else
+	{
       /* We must use SP as the base register, because SP is one of the
          registers being restored.  If an interrupt or page fault
          happens in the ldm instruction, the SP might or might not
@@ -14570,6 +14747,7 @@
 	asm_fprintf (f, "\tsub\t%r, %r, #%d\n", SP_REGNUM, FP_REGNUM,
 		     4 * bit_count (saved_regs_mask));
       print_multi_reg (f, "ldmfd\t%r, ", SP_REGNUM, saved_regs_mask, 0);
+	}
 
       if (IS_INTERRUPT (func_type))
 	/* Interrupt handlers will have pushed the
@@ -14598,8 +14776,19 @@
       unsigned HOST_WIDE_INT amount;
       int rfe;
       /* Restore stack pointer if necessary.  */
-      if (TARGET_ARM && frame_pointer_needed)
+      if (arm_abi == ARM_ABI_APCS32)
 	{
+	  if (abs (offsets->frame_size) + offsets->outgoing_args)
+	    {
+	      operands[0] = operands[1] = stack_pointer_rtx;
+	      operands[2] = GEN_INT (abs (offsets->frame_size)
+					  + offsets->outgoing_args);
+	      output_add_immediate (operands);
+	    }
+	}
+      else
+      if (TARGET_ARM && arm_apcs_frame_needed ())
+	{
 	  operands[0] = stack_pointer_rtx;
 	  operands[1] = hard_frame_pointer_rtx;
 	  
@@ -14608,7 +14797,7 @@
 	}
       else
 	{
-	  if (frame_pointer_needed)
+	  if (arm_apcs_frame_needed ())
 	    {
 	      /* For Thumb-2 restore sp from the frame pointer.
 		 Operand restrictions mean we have to incrememnt FP, then copy
@@ -14656,7 +14845,7 @@
 	      operands[2] = GEN_INT (amount);
 	      output_add_immediate (operands);
 	    }
-	  if (frame_pointer_needed)
+	  if (arm_apcs_frame_needed ())
 	    asm_fprintf (f, "\tmov\t%r, %r\n",
 			 SP_REGNUM, HARD_FRAME_POINTER_REGNUM);
 	}
@@ -14857,8 +15046,11 @@
 
       gcc_assert (!use_return_insn (FALSE, NULL)
 		  || (cfun->machine->return_used_this_function != 0)
-		  || offsets->saved_regs == offsets->outgoing_args
-		  || frame_pointer_needed);
+		  || (arm_abi != ARM_ABI_APCS32
+		      && offsets->saved_regs == offsets->outgoing_args)
+		  || (arm_abi == ARM_ABI_APCS32
+		      && abs (offsets->frame_size + offsets->outgoing_args) <= 256)
+		  || arm_apcs_frame_needed ());
 
       /* Reset the ARM-specific per-function variables.  */
       after_arm_reorg = 0;
@@ -15173,15 +15365,23 @@
 
   /* Initially this is the size of the local variables.  It will translated
      into an offset once we have determined the size of preceding data.  */
+#ifndef TARGET_RISCOSELF
   frame_size = ROUND_UP_WORD (get_frame_size ());
+#else
+  offsets->frame_size = frame_size = ROUND_UP_WORD (get_frame_size ());
+#endif
 
+#ifndef TARGET_RISCOSELF
   leaf = leaf_function_p ();
+#else
+  leaf = arm_leaf_function_p ();
+#endif
 
   /* Space for variadic functions.  */
   offsets->saved_args = crtl->args.pretend_args_size;
 
   /* In Thumb mode this is incorrect, but never used.  */
-  offsets->frame = offsets->saved_args + (frame_pointer_needed ? 4 : 0) +
+  offsets->frame = offsets->saved_args + (arm_apcs_frame_needed () ? 4 : 0) +
                    arm_compute_static_chain_stack_bytes();
 
   if (TARGET_32BIT)
@@ -15234,7 +15434,7 @@
   offsets->soft_frame = offsets->saved_regs + CALLER_INTERWORKING_SLOT_SIZE;
   /* A leaf function does not need any stack alignment if it has nothing
      on the stack.  */
-  if (leaf && frame_size == 0)
+  if (leaf && frame_size == 0 && arm_abi != ARM_ABI_APCS32)
     {
       offsets->outgoing_args = offsets->soft_frame;
       offsets->locals_base = offsets->soft_frame;
@@ -15280,7 +15480,10 @@
 	}
     }
 
-  offsets->locals_base = offsets->soft_frame + frame_size;
+  if (arm_abi == ARM_ABI_APCS32)
+    offsets->locals_base = 0;
+  else
+    offsets->locals_base = offsets->soft_frame + frame_size;
   offsets->outgoing_args = (offsets->locals_base
 			    + crtl->outgoing_args_size);
 
@@ -15306,6 +15509,29 @@
 
   offsets = arm_get_frame_offsets ();
 
+  if (arm_abi == ARM_ABI_APCS32)
+    {
+      if (from == ARG_POINTER_REGNUM || to == ARG_POINTER_REGNUM)
+	{
+	  int stack_frame = arm_apcs_frame_needed () ? 16 : 0;
+	  /*printf ("elim: outgoing=%d, saved_regs=%d, frame_size=%d, saved_args=%d, stack_frame=%d\n",
+		  offsets->outgoing_args, offsets->saved_regs,
+		  offsets->frame_size, offsets->saved_args, stack_frame); */
+	  return (offsets->outgoing_args
+		  + (offsets->saved_regs - offsets->saved_args)
+		  + stack_frame
+		  + offsets->frame_size - 4);
+	}
+
+      /* return (offsets->outgoing_args + offsets->saved_regs
+	 + offsets->frame - 4); */
+  
+      if (from == FRAME_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
+	return 0;
+  
+      abort ();
+    }
+
   /* OK, now we have enough information to compute the distances.
      There must be an entry in these switch tables for each pair
      of registers in ELIMINABLE_REGS, even if some of the entries
@@ -15382,13 +15608,116 @@
 bool
 arm_can_eliminate (const int from, const int to)
 {
+  if (arm_abi == ARM_ABI_APCS32)
+    {
+      /* We can eliminate ARGP to STACKP if no alloca, no stack checks needed
+	 and frame not needed.  */
+      if (from == ARG_POINTER_REGNUM && to == STACK_POINTER_REGNUM
+	  && ! arm_apcs_frame_needed ())
+	return true;
+  
+      /* FRAMEP can be eliminated to STACKP.  */
+      if (from == FRAME_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
+	return true;
+  
+      /* Can't do any other eliminations.  */
+      return false;
+    }
+
   return ((to == FRAME_POINTER_REGNUM && from == ARG_POINTER_REGNUM) ? false :
-          (to == STACK_POINTER_REGNUM && frame_pointer_needed) ? false :
+          (to == STACK_POINTER_REGNUM && arm_apcs_frame_needed ()) ? false :
           (to == ARM_HARD_FRAME_POINTER_REGNUM && TARGET_THUMB) ? false :
           (to == THUMB_HARD_FRAME_POINTER_REGNUM && TARGET_ARM) ? false :
            true);
 }
+ 
+/* Return non-zero if this is a leaf function.  */
+static int
+arm_leaf_function_p (void)
+{
+#ifdef TARGET_RISCOSELF
+  /* This function is designed to cache the result of leaf_function_p()
+     on the first call and then return the cached result on future calls.
+     However, it would seem that the first call isn't always correct and
+     as this result is used to determine whether stack checking is
+     required, we end up with non-leaf functions without any stack
+     checking at all. If a number of these functions call each other, it
+     leads to a corrupt stack.
+     We could forget the cached result (as newer versions of GCC do) and
+     just call leaf_function_p() every time, but the result tends to change
+     constantly for the same function while GCC rearranges code and
+     optimises things away, etc, and of course right at the crucial point
+     where the need for stack checking is being determined, we get the
+     wrong result.
+     So I've used a more conservative approach. After initially deciding
+     that a function is leaf, GCC can change its mind and change a leaf
+     function into a non-leaf function, but once it becomes a non-leaf
+     function, it stays like that. This may lead to some leaf functions
+     gaining a stack frame and stack check when they don't need them, but
+     at least we shouldn't get non-leaf functions without stack checks.  */
+  int leaf = leaf_function_p ();
+  
+  if (cfun->machine->leaf < 0 || (cfun->machine->leaf == 1 && leaf == 0))
+    cfun->machine->leaf = leaf;
+#else
+  if (cfun->machine->leaf < 0)
+    cfun->machine->leaf = leaf_function_p ();
+#endif
+  return cfun->machine->leaf;
+}
 
+/* Return 1 if the function prologue should contain an explicit
+   stack check.  */
+static bool
+arm_stack_check_needed (void)
+{
+  int frame_size;
+
+  /* Don't do any stack checking if it was not asked for.  */
+  if (!OPTION_APCS_STACK)
+    return false;
+  
+  /* We will always use stack checking for non-optimising
+     circumstances.  */
+  if (! optimize)
+    return true;
+
+  frame_size = ROUND_UP_WORD (get_frame_size ());
+  
+  /* Don't do any stack checking if the function is a leaf function
+     and the amount of stack actually needed <= 256 bytes.  */
+  if (arm_leaf_function_p () && abs (frame_size) <= 256)
+    return false;
+  
+  return true;
+}
+
+/* Return 1 if the function prologue really needs to setup an APCS
+   frame.  */
+bool
+arm_apcs_frame_needed (void)
+{
+  /* If we are not targeting the APCS, we will not use a stack frame.  */
+  if (! TARGET_APCS_FRAME)
+    return false;
+
+  /* If we are not optimising, or we call alloca, we will always
+     setup a frame.  */
+  if (cfun->calls_alloca
+      || crtl->has_nonlocal_goto
+      || cfun->has_nonlocal_label
+      || ! optimize)
+    return true;
+
+  /* A frame will need to be setup for the cases where there are external
+     function calls within the current function or there is a need
+     for definite stack checking.  */
+  if (! arm_leaf_function_p () || arm_stack_check_needed ())
+    return true;
+
+  return false;
+}
+
 /* Emit RTL to save coprocessor registers on function entry.  Returns the
    number of bytes pushed.  */
 
@@ -15525,7 +15854,9 @@
 void
 arm_expand_prologue (void)
 {
+#ifndef TARGET_RISCOSELF
   rtx amount;
+#endif
   rtx insn;
   rtx ip_rtx;
   unsigned long live_regs_mask;
@@ -15533,6 +15864,7 @@
   int fp_offset = 0;
   int saved_pretend_args = 0;
   int saved_regs = 0;
+  int frame_size;
   unsigned HOST_WIDE_INT args_to_push;
   arm_stack_offsets *offsets;
 
@@ -15542,6 +15874,16 @@
   if (IS_NAKED (func_type))
     return;
 
+#ifdef TARGET_RISCOSELF
+  /* FIXME: this cause an assert in require_pic_register().  */
+  if (0 && flag_pic == 2 && arm_apcs_frame_needed())
+    {
+      /* If there's a stack extension call, then we need the PIC
+         register setting up before it for a call via the PLT.  */
+      require_pic_register ();
+    }
+#endif
+
   /* Make a copy of c_f_p_a_s as we may need to modify it locally.  */
   args_to_push = crtl->args.pretend_args_size;
 
@@ -15586,7 +15928,7 @@
   /* For APCS frames, if IP register is clobbered
      when creating frame, save that register in a special
      way.  */
-  if (TARGET_APCS_FRAME && frame_pointer_needed && TARGET_ARM)
+  if (TARGET_APCS_FRAME && arm_apcs_frame_needed () && TARGET_ARM)
     {
       if (IS_INTERRUPT (func_type))
 	{
@@ -15693,7 +16035,7 @@
      can be done with a single instruction.  */
   if ((func_type == ARM_FT_ISR || func_type == ARM_FT_FIQ)
       && (live_regs_mask & (1 << LR_REGNUM)) != 0
-      && !(frame_pointer_needed && TARGET_APCS_FRAME)
+      && !(arm_apcs_frame_needed () && TARGET_APCS_FRAME)
       && TARGET_ARM)
     {
       rtx lr = gen_rtx_REG (SImode, LR_REGNUM);
@@ -15704,7 +16046,7 @@
   if (live_regs_mask)
     {
       saved_regs += bit_count (live_regs_mask) * 4;
-      if (optimize_size && !frame_pointer_needed
+      if (optimize_size && !arm_apcs_frame_needed ()
 	  && saved_regs == offsets->saved_regs - offsets->saved_args)
 	{
 	  /* If no coprocessor registers are being pushed and we don't have
@@ -15731,7 +16073,17 @@
   if (! IS_VOLATILE (func_type))
     saved_regs += arm_save_coproc_regs ();
 
-  if (frame_pointer_needed && TARGET_ARM)
+  if (arm_abi == ARM_ABI_APCS32)
+    frame_size = - offsets->frame_size - offsets->outgoing_args;
+  else
+    frame_size = offsets->saved_args + saved_regs - offsets->outgoing_args;
+
+#ifdef TARGET_RISCOSELF
+  if (flag_pic == 2)
+    arm_load_pic_register (0UL);
+#endif
+
+  if (arm_apcs_frame_needed () && TARGET_ARM)
     {
       /* Create the new frame pointer.  */
       if (TARGET_APCS_FRAME)
@@ -15740,6 +16092,48 @@
 	  insn = emit_insn (gen_addsi3 (hard_frame_pointer_rtx, ip_rtx, insn));
 	  RTX_FRAME_RELATED_P (insn) = 1;
 
+#if defined(ARM_STKOVF_SPLIT_SMALL) && defined(ARM_STKOVF_SPLIT_BIG)
+      /* Explicit stack checks.  */
+      if (arm_stack_check_needed ())
+	{
+	  rtx last = get_last_insn ();
+	  rtx sl_reg = gen_rtx_REG (GET_MODE (stack_pointer_rtx), SL_REGNUM);
+	  if (frame_size <= -256)
+	    {
+	      rtx stkovf = gen_rtx_SYMBOL_REF (Pmode, ARM_STKOVF_SPLIT_BIG);
+	      emit_insn (gen_addsi3 (ip_rtx, stack_pointer_rtx,
+				     GEN_INT (frame_size)));
+	      if (call_used_regs[8])
+		{
+		  rtx null = gen_rtx_SYMBOL_REF(Pmode, "__ABC__");
+		  insn = emit_insn (gen_rt_stkovf_v5_clobbered (ip_rtx, sl_reg, stkovf, null));
+		}
+	      else
+		insn = emit_insn (gen_rt_stkovf (ip_rtx, sl_reg, stkovf));
+	    }
+	  else
+	    {
+	      rtx stkovf = gen_rtx_SYMBOL_REF (Pmode, ARM_STKOVF_SPLIT_SMALL);
+	      if (call_used_regs[8])
+		{
+		  rtx null = gen_rtx_SYMBOL_REF(Pmode, "__ABC__");
+		  insn = emit_insn (gen_rt_stkovf_v5_clobbered (stack_pointer_rtx, sl_reg, stkovf, null));
+		}
+	      else
+	        insn = emit_insn (gen_rt_stkovf (stack_pointer_rtx, sl_reg, stkovf));
+	    }
+	  /* Create barrier to prevent real stack adjustment from being
+	     scheduled before call to stack checker.  */
+	  emit_insn (gen_blockage ());
+	  do
+	    {
+	      last = last ? NEXT_INSN(last) : get_insns ();
+	      RTX_FRAME_RELATED_P (last) = 1;
+	    }
+	  while (last != insn);
+	}
+#endif
+
 	  if (IS_NESTED (func_type))
 	    {
 	      /* Recover the static chain register.  */
@@ -15765,17 +16159,14 @@
 	}
     }
 
-  if (offsets->outgoing_args != offsets->saved_args + saved_regs)
+  if (frame_size != 0)
     {
       /* This add can produce multiple insns for a large constant, so we
 	 need to get tricky.  */
       rtx last = get_last_insn ();
 
-      amount = GEN_INT (offsets->saved_args + saved_regs
-			- offsets->outgoing_args);
-
       insn = emit_insn (gen_addsi3 (stack_pointer_rtx, stack_pointer_rtx,
-				    amount));
+				    GEN_INT (frame_size)));
       do
 	{
 	  last = last ? NEXT_INSN (last) : get_insns ();
@@ -15786,16 +16177,29 @@
       /* If the frame pointer is needed, emit a special barrier that
 	 will prevent the scheduler from moving stores to the frame
 	 before the stack adjustment.  */
-      if (frame_pointer_needed)
+      if (arm_apcs_frame_needed ())
 	insn = emit_insn (gen_stack_tie (stack_pointer_rtx,
 					 hard_frame_pointer_rtx));
+
+      if (arm_abi == ARM_ABI_APCS32
+	  && (!optimize
+	      || (cfun->calls_alloca && !OPTION_APCS_STACK)))
+	{
+	  /* These are the cases when we still have an uneliminated
+	     FRAME_POINTER_REGNUM usage: either in an unoptimized case, either
+	     when the function calls alloca and we don't have APCS stack
+	     checking which results in directly allocating alloca memory
+	     on the stack.  */
+	  insn = emit_insn (gen_movsi (frame_pointer_rtx, stack_pointer_rtx));
+	  RTX_FRAME_RELATED_P (insn) = 1;
+	}
     }
 
 
-  if (frame_pointer_needed && TARGET_THUMB2)
+  if (arm_apcs_frame_needed () && TARGET_THUMB2)
     thumb_set_frame_pointer (offsets);
 
-  if (flag_pic && arm_pic_register != INVALID_REGNUM)
+  if ((flag_pic != 2 || TARGET_MODULE) && arm_pic_register != INVALID_REGNUM)
     {
       unsigned long mask;
 
@@ -16699,6 +17103,11 @@
       if (NEED_GOT_RELOC && flag_pic && making_const_table &&
 	  (GET_CODE (x) == SYMBOL_REF || GET_CODE (x) == LABEL_REF))
 	{
+	  if (TARGET_MODULE) /* -mmodule */
+ 	    fputs ("(GOTOFF)", asm_out_file);
+	  else if (flag_pic == 2) /* -fPIC */
+ 	    fputs ("(GOT)", asm_out_file);
+	  else /* -fpic */
 	  /* See legitimize_pic_address for an explanation of the
 	     TARGET_VXWORKS_RTP check.  */
 	  if (TARGET_VXWORKS_RTP
@@ -16787,7 +17196,11 @@
 static void
 arm_elf_asm_constructor (rtx symbol, int priority)
 {
+#ifdef TARGET_RISCOSELF
+  default_named_section_asm_out_constructor (symbol, priority);
+#else
   arm_elf_asm_cdtor (symbol, priority, /*is_ctor=*/true);
+#endif
 }
 
 /* Add a function to the list of static destructors.  */
@@ -17537,7 +17950,7 @@
   /* If we are using the stack pointer to point at the
      argument, then an offset of 0 is correct.  */
   /* ??? Check this is consistent with thumb2 frame layout.  */
-  if ((TARGET_THUMB || !frame_pointer_needed)
+  if ((TARGET_THUMB || !arm_apcs_frame_needed ())
       && REGNO (addr) == SP_REGNUM)
     return 0;
 
@@ -20217,7 +20630,7 @@
   unsigned long high_regs_pushed = bit_count (live_regs_mask & 0x0f00);
   int n_free, reg_base;
 
-  if (!for_prologue && frame_pointer_needed)
+  if (!for_prologue && arm_apcs_frame_needed ())
     amount = offsets->locals_base - offsets->saved_regs;
   else
     amount = offsets->outgoing_args - offsets->saved_regs;
@@ -20247,7 +20660,7 @@
      between the push and the stack frame allocation.  */
   if (for_prologue
       && ((flag_pic && arm_pic_register != INVALID_REGNUM)
-	  || (!frame_pointer_needed && CALLER_INTERWORKING_SLOT_SIZE > 0)))
+	  || (!arm_apcs_frame_needed () && CALLER_INTERWORKING_SLOT_SIZE > 0)))
     return 0;
 
   reg_base = 0;
@@ -20457,6 +20870,7 @@
 #if ARM_FT_UNKNOWN != 0
   machine->func_type = ARM_FT_UNKNOWN;
 #endif
+  machine->leaf = -1;
   return machine;
 }
 
@@ -20468,6 +20882,21 @@
   if (count != 0)
     return NULL_RTX;
 
+  if (arm_abi == ARM_ABI_APCS32)
+    {
+      /* On RISC OS, the story is more complicated because calls to
+	 stack-extension code, or alloca can directly alter the
+	 return address of the current function.  We rely on the
+         target's run-time C library to provide an appropriate solution.  */
+      rtx function_call = gen_rtx_SYMBOL_REF (Pmode,
+					      "__builtin_return_address");
+      frame = gen_reg_rtx (Pmode);
+      emit_library_call_value (function_call, frame, LCT_NORMAL,
+			       Pmode, 1,
+			       GEN_INT (count), SImode);
+      return frame;
+    }
+
   return get_hard_reg_initial_val (Pmode, LR_REGNUM);
 }
 
@@ -20573,7 +21002,7 @@
   if (flag_pic && arm_pic_register != INVALID_REGNUM)
     arm_load_pic_register (live_regs_mask);
 
-  if (!frame_pointer_needed && CALLER_INTERWORKING_SLOT_SIZE > 0)
+  if (!arm_apcs_frame_needed () && CALLER_INTERWORKING_SLOT_SIZE > 0)
     emit_move_insn (gen_rtx_REG (Pmode, ARM_HARD_FRAME_POINTER_REGNUM),
 		    stack_pointer_rtx);
 
@@ -20626,7 +21055,7 @@
 	}
     }
 
-  if (frame_pointer_needed)
+  if (arm_apcs_frame_needed ())
     thumb_set_frame_pointer (offsets);
 
   /* If we are profiling, make sure no instructions are scheduled before
@@ -20659,7 +21088,7 @@
   offsets = arm_get_frame_offsets ();
   amount = offsets->outgoing_args - offsets->saved_regs;
 
-  if (frame_pointer_needed)
+  if (arm_apcs_frame_needed ())
     {
       emit_insn (gen_movsi (stack_pointer_rtx, hard_frame_pointer_rtx));
       amount = offsets->locals_base - offsets->saved_regs;
@@ -21975,7 +22404,7 @@
     emit_move_insn (gen_rtx_REG (Pmode, LR_REGNUM), source);
   else
     {
-      if (frame_pointer_needed)
+      if (arm_apcs_frame_needed ())
 	addr = plus_constant(hard_frame_pointer_rtx, -4);
       else
 	{
@@ -22018,7 +22447,7 @@
     {
       limit = 1024;
       /* Find the saved regs.  */
-      if (frame_pointer_needed)
+      if (arm_apcs_frame_needed ())
 	{
 	  delta = offsets->soft_frame - offsets->saved_args;
 	  reg = THUMB_HARD_FRAME_POINTER_REGNUM;
@@ -22147,7 +22576,92 @@
   return mode == SImode ? 255 : 0;
 }
 
+void
+arm_expand_save_stack_block (rtx save_area, rtx stack_pointer ATTRIBUTE_UNUSED)
+{
+  emit_library_call_value (gen_rtx_SYMBOL_REF (Pmode, "__gcc_alloca_save"),
+                           save_area, LCT_NORMAL, GET_MODE (save_area), 0);
+}
 
+void
+arm_expand_restore_stack_block (rtx stack_pointer ATTRIBUTE_UNUSED, rtx save_area)
+{
+  emit_library_call (gen_rtx_SYMBOL_REF (Pmode, "__gcc_alloca_restore"),
+		     LCT_NORMAL, VOIDmode,
+		     2,
+		     gen_rtx_REG (SImode, FP_REGNUM), SImode,
+		     save_area, Pmode);
+}
+
+void
+arm_expand_alloca_epilogue (void)
+{
+  rtx sym = gen_rtx_SYMBOL_REF (Pmode, "__gcc_alloca_free");
+#if 1
+  emit_insn (gen_call_no_clobber (sym));
+#else
+  emit_library_call (sym,
+		     LCT_NORMAL, VOIDmode,
+		     0);
+#endif
+}
+
+void
+arm_expand_allocate_stack (rtx memptr, rtx size)
+{
+  emit_library_call_value (gen_rtx_SYMBOL_REF (Pmode, "__gcc_alloca"),
+                           memptr, LCT_NORMAL, GET_MODE (memptr),
+                      	   1,
+			   size, SImode);
+}
+
+void
+arm_expand_save_stack_nonlocal (rtx *operands)
+{
+  rtx sa = XEXP (operands[0], 0);
+  /*rtx sp = operands[1];*/
+
+  /*emit_move_insn (sa, gen_rtx_REG (Pmode, FP_REGNUM));*/
+  emit_move_insn (gen_rtx_MEM (Pmode, plus_constant (sa, 0)),
+                  gen_rtx_REG (Pmode, FP_REGNUM));
+
+  /*emit_move_insn (gen_rtx_MEM (Pmode, plus_constant (sa, 4)), sp); */
+}
+
+
+void
+arm_expand_restore_stack_nonlocal (rtx *operands)
+{
+  rtx sa = XEXP (operands[1], 0);
+  rtx sp = operands[0];
+  rtx fp = gen_rtx_REG (Pmode, FP_REGNUM);
+
+  emit_insn (gen_rtx_SET (VOIDmode, sp, fp));
+  emit_insn (gen_rtx_SET (VOIDmode, fp,
+                          gen_rtx_MEM (Pmode, plus_constant (sa, 0))));
+
+  /*  emit_move_insn (gen_rtx_REG (Pmode, FP_REGNUM),
+      gen_rtx_MEM (Pmode, plus_constant (sa, 0)));
+      emit_move_insn (sp, gen_rtx_MEM (Pmode, plus_constant (sa, 4)));*/
+
+  /* In non-optimising circumstances, ensure we make v6 = sp */
+  if (! optimize)
+    emit_move_insn (frame_pointer_rtx, sp);
+}
+
+void
+arm_expand_nonlocal_goto (rtx *operands)
+{
+  rtx sa = XEXP (operands[2], 0);
+  rtx fp = gen_rtx_REG (Pmode, FP_REGNUM);
+
+  emit_move_insn (gen_rtx_MEM (Pmode, plus_constant (sa, 0)), fp);
+  emit_move_insn (gen_rtx_MEM (Pmode, plus_constant (sa, 4)),
+                  stack_pointer_rtx);
+  emit_indirect_jump (operands[1]);
+}
+
+
 /* Map internal gcc register numbers to DWARF2 register numbers.  */
 
 unsigned int
@@ -22161,6 +22675,9 @@
   if (IS_FPA_REGNUM (regno))
     return (TARGET_AAPCS_BASED ? 96 : 16) + regno - FIRST_FPA_REGNUM;
 
+  if (IS_CIRRUS_REGNUM (regno))
+    return 28 + regno - FIRST_CIRRUS_FP_REGNUM;
+
   if (IS_VFP_REGNUM (regno))
     {
       /* See comment in arm_dwarf_register_span.  */
@@ -22176,7 +22693,13 @@
   if (IS_IWMMXT_REGNUM (regno))
     return 112 + regno - FIRST_IWMMXT_REGNUM;
 
-  gcc_unreachable ();
+  /* FIXME: GCCSDK: It is very odd we have to make an exception on pseudo registers
+     24, 25, 26 and anything >= FIRST_HI_VFP_REGNUM as otherwise we reach
+     gcc_unreachable() when being called from expand_builtin_init_dwarf_reg_sizes()
+     which is enumerating all registers from 0 until FIRST_PSEUDO_REGISTER.  Why
+     don't we have this problem in other ARM targets ? */
+  /* gcc_unreachable (); */
+  return 0;
 }
 
 /* Dwarf models VFPv3 registers as 32 64-bit registers.
@@ -22899,6 +23422,10 @@
 bool
 arm_frame_pointer_required (void)
 {
+  /* The APCS-32 ABI never requires a frame pointer. */
+  if (arm_abi == ARM_ABI_APCS32)
+    return false;
+
   return (cfun->has_nonlocal_label
           || SUBTARGET_FRAME_POINTER_REQUIRED
           || (TARGET_ARM && TARGET_APCS_FRAME && ! leaf_function_p ()));
@@ -23401,10 +23928,10 @@
       fixed_regs[PIC_OFFSET_TABLE_REGNUM] = 1;
       call_used_regs[PIC_OFFSET_TABLE_REGNUM] = 1;
     }
-  else if (TARGET_APCS_STACK)
+  if (OPTION_APCS_STACK)
     {
-      fixed_regs[10]     = 1;
-      call_used_regs[10] = 1;
+      fixed_regs[SL_REGNUM]     = 1;
+      call_used_regs[SL_REGNUM] = 1;
     }
   /* -mcaller-super-interworking reserves r11 for calls to
      _interwork_r11_call_via_rN().  Making the register global