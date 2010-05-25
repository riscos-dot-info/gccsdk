--- acorn/GMakefile.orig	2009-12-07 18:36:22.000000000 -0800
+++ acorn/GMakefile	2009-12-07 18:36:54.000000000 -0800
@@ -2,129 +2,101 @@
 
 # add -g to CC to debug
 # add -d to BIND to debug
-CC   = gcc -mlibscl
+CC   = gcc
 BIND = $(CC)
-AS   = $(CC) -c
-ASM  = AS
-SQUEEZE = squeeze -v
-E    =
+ASM  = $(CC) -mhard-float -xassembler-with-cpp -c
+E    = 
 
 # flags
 #   CFLAGS    flags for C compile
 #   LFLAGS1   flags after output file spec, before obj file list
 #   LFLAGS2   flags after obj file list (libraries, etc)
+#   ASMFLAGS  flags for assembler
 #
-LIB      =
-CFLAGS   = -O2 -mthrowback -DASMV
-ASMFLAGS = -throwback -objasm -upper
-LFLAGS1  = 
-LFLAGS2  = $(LIB)
+CFLAGS   = -mthrowback -mlibscl -I. -O2 -Wall
+LFLAGS1  = -mlibscl
+LFLAGS2  =
+ASMFLAGS = -mthrowback
 
 # Uncomment the following line to enable support for Unix
 # Extra Field (Timezone)
 #CFLAGS  = $(CFLAGS) -DUSE_EF_UT_TIME
 
 # object file lists
-OBJZ = o.zip o.zipfile o.zipup o.fileio o.util o.globals o.crc32 \
-       o.crypt o.ttyio o.riscos o.acornzip o.swiven
+OBJZ = zip.o zipfile.o zipup.o fileio.o util.o globals.o crypt.o ttyio.o \
+	crc32.o riscos.o acornzip.o swiven.o
 
-OBJI = o.deflate o.trees
-OBJA = o.match o.sendbits
-OBJU = o.zipfile_ o.fileio_ o.util_ o.globals o.riscos o.acornzip_ o.swiven
-OBJN = o.zipnote  $(OBJU)
-OBJC = o.zipcloak $(OBJU) o.crc32_ o.crypt_ o.ttyio
-OBJS = o.zipsplit $(OBJU)
-
-ZIP_H = h.zip h.ziperr h.tailor acorn.h.osdep acorn.h.riscos acorn.h.swiven
-
-all:      zip zipnote zipsplit zipcloak
-
-install:  %.zip %.zipnote %.zipsplit %.zipcloak %.acorn.zipsfx \
-          zip zipnote zipsplit zipcloak acorn.zipsfx
+OBJI = deflate.o trees.o
+OBJA = 
+OCRCU8 = 
+OCRCTB = crc32_.o
+OBJU = zipfile_.o fileio_.o util_.o globals.o riscos.o acornzip_.o swiven.o \
+	$(OCRCUB)
+OBJN = zipnote.o  $(OBJU)
+OBJC = zipcloak.o $(OBJU) $(OCRCTB) crypt_.o ttyio.o
+OBJS = zipsplit.o $(OBJU)
+
+ifndef NOASM
+OBJA = match.o sendbits.o
+CFLAGS += -DASMV
+endif
+
+ZIP_H = zip.h ziperr.h tailor.h acorn/osdep.h acorn/riscos.h acorn/swiven.h
+
+all:      zips
+
+VPATH = .:acorn
+
+# suffix rules
+.SUFFIXES:
+.SUFFIXES: _.o .o .c .s
+.c_.o:
+	$(CC) -c $(CFLAGS) -DUTIL -o $@ $<
+
+.c.o:
+	$(CC) -c $(CFLAGS) $<
+
+.s.o:
+	$(ASM) $(ASMFLAGS) -o $@ $<
+
+# rules for zip, zipnote, zipcloak, zipsplit, and the Zip MANUALs.
+$(OBJZ): $(ZIP_H)
+$(OBJI): $(ZIP_H)
+$(OBJN): $(ZIP_H)
+$(OBJS): $(ZIP_H)
+$(OBJC): $(ZIP_H)
+zip.o zipup.o zipfile.o fileio.o crc32.o crypt.o: crc32.h
+zipcloak.o zipfile_.o fileio_.o crc32_.o crypt_.o: crc32.h
+zip.o zipup.o crypt.o ttyio.o zipcloak.o crypt_.o: crypt.h
+zip.o zipup.o zipnote.o zipcloak.o zipsplit.o: revision.h
+zip.o crypt.o ttyio.o zipcloak.o crypt_.o: ttyio.h
+zipup.o: unix/zipup.h
+
+ZIPS = zip$E zipcloak$E zipnote$E zipsplit$E
+
+zips: $(ZIPS)
+
+zip$E: $(OBJZ) $(OBJI) $(OBJA) $(LIB_BZ)
+	$(BIND) -o zip$E $(LFLAGS1) $(OBJZ) $(OBJI) $(OBJA) $(LFLAGS2)
+zipnote$E: $(OBJN)
+	$(BIND) -o zipnote$E $(LFLAGS1) $(OBJN) $(LFLAGS2)
+zipcloak$E: $(OBJC) $(OCRCTB)
+	$(BIND) -o zipcloak$E $(LFLAGS1) $(OBJC) $(LFLAGS2)
+zipsplit$E: $(OBJS)
+	$(BIND) -o zipsplit$E $(LFLAGS1) $(OBJS) $(LFLAGS2)
+
+# install
+install:  %.zip %.zipnote %.zipsplit %.zipcloak \
+          zip zipnote zipsplit zipcloak acorn/zipsfx
 	$(SQUEEZE) zip %.zip
 	$(SQUEEZE) zipnote %.zipnote
 	$(SQUEEZE) zipsplit %.zipsplit
 	$(SQUEEZE) zipcloak %.zipcloak
 	copy acorn.zipsfx %.zipsfx ~CVF
 
-# rules for zip, zipnote, zipcloak and zipsplit
-
-o.api:		c.api
-	$(CC) $(CFLAGS) -c c.api -o o.api
-o.crc32:	c.crc32 $(ZIP_H) h.crc32
-	$(CC) $(CFLAGS) -c c.crc32 -o o.crc32
-o.crypt:	c.crypt $(ZIP_H) h.crypt h.crc32 h.ttyio
-	$(CC) $(CFLAGS) -c c.crypt -o o.crypt
-o.deflate:	c.deflate $(ZIP_H)
-	$(CC) $(CFLAGS) -c c.deflate -o o.deflate
-o.fileio:	c.fileio $(ZIP_H) h.crc32
-	$(CC) $(CFLAGS) -c c.fileio -o o.fileio
-o.globals:	c.globals $(ZIP_H)
-	$(CC) $(CFLAGS) -c c.globals -o o.globals
-o.mktime:	c.mktime
-	$(CC) $(CFLAGS) -c c.mktime -o o.mktime
-o.trees:	c.trees $(ZIP_H)
-	$(CC) $(CFLAGS) -c c.trees -o o.trees
-o.ttyio:	c.ttyio $(ZIP_H) h.crypt
-	$(CC) $(CFLAGS) -c c.ttyio -o o.ttyio
-o.util:		c.util $(ZIP_H)
-	$(CC) $(CFLAGS) -c c.util -o o.util
-o.zip:		c.zip $(ZIP_H) h.crc32 h.crypt h.revision h.ttyio
-	$(CC) $(CFLAGS) -c c.zip -o o.zip
-o.zipcloak:	c.zipcloak $(ZIP_H) h.crc32 h.crypt h.revision h.ttyio
-	$(CC) $(CFLAGS) -c c.zipcloak -o o.zipcloak
-o.zipfile:	c.zipfile $(ZIP_H) h.crc32
-	$(CC) $(CFLAGS) -c c.zipfile -o o.zipfile
-o.zipnote:	c.zipnote $(ZIP_H) h.revision
-	$(CC) $(CFLAGS) -c c.zipnote -o o.zipnote
-o.zipsplit:	c.zipsplit $(ZIP_H) h.revision
-	$(CC) $(CFLAGS) -c c.zipsplit -o o.zipsplit
-o.zipup:	c.zipup $(ZIP_H) h.crc32 h.crypt h.revision
-	$(CC) $(CFLAGS) -c c.zipup -o o.zipup
-
-o.crc32_: c.crc32 $(ZIP_H) h.crc32
-	$(CC) $(CFLAGS) -DUTIL -c c.crc32 -o o.crc32_
-o.crypt_: c.crypt $(ZIP_H) h.crypt h.crc32 h.ttyio
-	$(CC) $(CFLAGS) -DUTIL -c c.crypt -o o.crypt_
-o.util_: c.util $(ZIP_H)
-	$(CC) $(CFLAGS) -DUTIL -c c.util -o o.util_
-o.fileio_: c.fileio $(ZIP_H) h.crc32
-	$(CC) $(CFLAGS) -DUTIL -c c.fileio -o o.fileio_
-o.zipfile_: c.zipfile $(ZIP_H) h.crc32
-	$(CC) $(CFLAGS) -DUTIL -c c.zipfile -o o.zipfile_
-o.acornzip_: acorn.c.acornzip $(ZIP_H)
-	$(CC) $(CFLAGS) -I@ -DUTIL -c acorn.c.acornzip -o o.acornzip_
-
-o.riscos: acorn.c.riscos acorn.h.riscos $(ZIP_H)
-	$(CC) $(CFLAGS) -I@ -c acorn.c.riscos -o o.riscos
-
-o.acornzip: acorn.c.acornzip $(ZIP_H)
-	$(CC) $(CFLAGS) -I@ -c acorn.c.acornzip -o o.acornzip
-
-o.match: acorn.s.match
-	$(ASM) $(ASMFLAGS) -I@ acorn.s.match -o o.match
-
-o.sendbits: acorn.s.sendbits
-	$(ASM) $(ASMFLAGS) -I@ acorn.s.sendbits -o o.sendbits
-
-o.swiven: acorn.s.swiven
-	$(ASM) $(ASMFLAGS) -I@ acorn.s.swiven -o o.swiven
-
-zip:	$(OBJZ) $(OBJI) $(OBJA)
-	$(BIND) -o zip$(E) $(LFLAGS1) $(OBJZ) $(OBJI) $(OBJA) $(LFLAGS2)
-zipnote:  $(OBJN)
-	$(BIND) -o zipnote$(E) $(LFLAGS1) $(OBJN) $(LFLAGS2)
-zipcloak: $(OBJC)
-	$(BIND) -o zipcloak$(E) $(LFLAGS1) $(OBJC) $(LFLAGS2)
-zipsplit: $(OBJS)
-	$(BIND) -o zipsplit$(E) $(LFLAGS1) $(OBJS) $(LFLAGS2)
-
-clean:	
-	remove zip
-	remove zipcloak	
-	remove zipsplit
-	remove zipnote
-	create o.!fake! 0
-	wipe o.* ~cf
+# clean up after making stuff and installing it
+clean:
+	rm -f *.o
+	rm -f *ff8 *e1f
 
 # end of Makefile