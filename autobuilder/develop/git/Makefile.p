--- Makefile.orig	2015-11-15 15:01:11.482581000 +0100
+++ Makefile	2015-11-15 15:01:28.998581000 +0100
@@ -1438,7 +1438,7 @@
 
 ifdef HAVE_CLOCK_GETTIME
 	BASIC_CFLAGS += -DHAVE_CLOCK_GETTIME
-	EXTLIBS += -lrt
+#	EXTLIBS += -lrt
 endif
 
 ifdef HAVE_CLOCK_MONOTONIC
