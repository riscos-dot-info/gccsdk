Index: configure.ac
===================================================================
--- configure.ac	(revision 167096)
+++ configure.ac	(working copy)
@@ -190,6 +190,7 @@
 target_libraries="target-libgcc \
 		target-libiberty \
 		target-libgloss \
+		target-libunixlib \
 		target-newlib \
 		target-libgomp \
 		target-libstdc++-v3 \
@@ -1036,6 +1037,14 @@
     ;;
 esac
 
+case "${target}" in
+  arm-riscos*-* | arm-*-riscos)
+    ;;
+  *)
+    noconfigdirs="$noconfigdirs target-libunixlib"
+    ;;
+esac
+
 # If we aren't building newlib, then don't build libgloss, since libgloss
 # depends upon some newlib header files.
 case "${noconfigdirs}" in
@@ -3016,6 +3025,10 @@
    ;;
   esac
   ;;
+
+ *" libunixlib "*)
+  FLAGS_FOR_TARGET=$FLAGS_FOR_TARGET' -B$$r/$(TARGET_SUBDIR)/libunixlib/ -B$$r/$(TARGET_SUBDIR)/libunixlib/.libs/ -isystem $$s/libunixlib/include'
+  ;;
 esac
 
 case "$target" in