--- Makefile.in.orig	2013-10-31 15:40:42.685866712 +0000
+++ Makefile.in	2013-10-31 15:41:07.397289084 +0000
@@ -212,7 +212,7 @@
 CSCOPE = cscope
 DIST_SUBDIRS = fontconfig fc-case fc-lang fc-glyphname src fc-cache \
 	fc-cat fc-list fc-match fc-pattern fc-query fc-scan \
-	fc-validate conf.d test doc
+	fc-validate conf.d doc
 DISTFILES = $(DIST_COMMON) $(DIST_SOURCES) $(TEXINFOS) $(EXTRA_DIST)
 distdir = $(PACKAGE)-$(VERSION)
 top_distdir = $(distdir)
@@ -414,7 +414,7 @@
 top_srcdir = @top_srcdir@
 SUBDIRS = fontconfig fc-case fc-lang fc-glyphname src fc-cache fc-cat \
 	fc-list fc-match fc-pattern fc-query fc-scan fc-validate \
-	conf.d test $(am__append_1)
+	conf.d $(am__append_1)
 ACLOCAL_AMFLAGS = -I m4
 EXTRA_DIST = fontconfig.pc.in fonts.conf.in fonts.dtd \
 	fontconfig.spec.in fontconfig.spec fontconfig-zip.in \