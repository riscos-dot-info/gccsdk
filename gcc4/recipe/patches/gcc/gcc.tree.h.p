From 31502f9f1f9095aaa60094385a2e116b03b23804 Mon Sep 17 00:00:00 2001
From: Jakub Jelinek <jakub@redhat.com>
Date: Tue, 5 Feb 2013 16:56:20 +0100
Subject: [PATCH 1/1] * tree.h (struct tree_decl_with_vis): Remove thread_local
 field.

From-SVN: r195762
---
 gcc/ChangeLog | 2 ++
 gcc/tree.h    | 7 +++----
 2 files changed, 5 insertions(+), 4 deletions(-)

diff --git a/gcc/tree.h b/gcc/tree.h
index 3bc39931d27c..c3c814ca33af 100644
--- gcc/tree.h
+++ gcc/tree.h
@@ -3198,13 +3198,14 @@ struct GTY(()) tree_decl_with_vis {
  /* Belong to VAR_DECL exclusively.  */
  unsigned defer_output : 1;
  unsigned hard_register : 1;
- unsigned thread_local : 1;
  unsigned common_flag : 1;
  unsigned in_text_section : 1;
  unsigned in_constant_pool : 1;
  unsigned dllimport_flag : 1;
  /* Don't belong to VAR_DECL exclusively.  */
  unsigned weak_flag : 1;
+ /* When SECTION_NAME is implied by -ffunction-section.  */
+ unsigned implicit_section_name_p : 1;
 
  unsigned seen_in_bind_expr : 1;
  unsigned comdat_flag : 1;
@@ -3217,9 +3218,7 @@ struct GTY(()) tree_decl_with_vis {
  unsigned init_priority_p : 1;
  /* Used by C++ only.  Might become a generic decl flag.  */
  unsigned shadowed_for_var_p : 1;
- /* When SECTION_NAME is implied by -ffunsection-section.  */
- unsigned implicit_section_name_p : 1;
- /* 13 unused bits. */
+ /* 14 unused bits. */
 };
 
 extern tree decl_debug_expr_lookup (tree);
-- 
2.43.7

