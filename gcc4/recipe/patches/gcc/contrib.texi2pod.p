From 386c402747d2bc6bad3e1bc2f9383d4d669576e0 Mon Sep 17 00:00:00 2001
From: Uros Bizjak <uros@gcc.gnu.org>
Date: Sat, 24 May 2014 08:38:31 +0200
Subject: [PATCH] texi2pod.pl: Force .pod file to not be a numbered list.

	* texi2pod.pl: Force .pod file to not be a numbered list.

From-SVN: r210889
---
 contrib/texi2pod.pl |  4 ++--
 1 file changed, 2 insertions(+), 2 deletions(-)

diff --git a/contrib/texi2pod.pl b/contrib/texi2pod.pl
index 5a4bbacdf5e3..55b6ba752279 100755
--- contrib/texi2pod.pl
+++ contrib/texi2pod.pl
@@ -1,6 +1,6 @@
 #! /usr/bin/perl -w
 
-#   Copyright (C) 1999, 2000, 2001, 2003, 2010 Free Software Foundation, Inc.
+#   Copyright (C) 1999-2014 Free Software Foundation, Inc.
 
 # This file is part of GCC.
 
@@ -337,7 +337,7 @@ while(<$inf>) {
                 $_ = "\n=item $1\n";
             }
 	} else {
-	    $_ = "\n=item $ic\n";
+	    $_ = "\n=item Z\&LT;\&GT;$ic\n";
 	    $ic =~ y/A-Ya-y/B-Zb-z/;
 	    $ic =~ s/(\d+)/$1 + 1/eg;
 	}
-- 
2.43.7

