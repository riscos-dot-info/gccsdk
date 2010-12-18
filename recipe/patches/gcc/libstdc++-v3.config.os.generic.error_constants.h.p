Index: libstdc++-v3/config/os/generic/error_constants.h
===================================================================
--- libstdc++-v3/config/os/generic/error_constants.h	(revision 162285)
+++ libstdc++-v3/config/os/generic/error_constants.h	(working copy)
@@ -94,7 +94,10 @@
       no_message_available = 			ENODATA, 
 #endif
 
-      no_message = 				ENOMSG, 
+#ifdef _GLIBCXX_HAVE_ENOMSG
+      no_message = 				ENOMSG,
+#endif
+
       no_protocol_option = 			ENOPROTOOPT,
       no_space_on_device = 			ENOSPC,
 