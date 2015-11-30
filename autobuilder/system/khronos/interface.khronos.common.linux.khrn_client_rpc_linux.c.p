--- interface/khronos/common/linux/khrn_client_rpc_linux.c.orig	2015-06-24 17:12:03.000000000 +0100
+++ interface/khronos/common/linux/khrn_client_rpc_linux.c	2015-07-14 14:29:58.953998171 +0100
@@ -36,6 +36,9 @@
 
 #include <string.h>
 #include <stdio.h>
+#include <kernel.h>
+#include "main.h"
+#include "../inline_swis.h"
 
 extern VCOS_LOG_CAT_T khrn_client_log;
 
@@ -57,48 +60,88 @@
 
 static VCOS_EVENT_T bulk_event;
 
-VCHIQ_STATUS_T khrn_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
-                  VCHIQ_SERVICE_HANDLE_T handle, void *bulk_userdata)
+static VCHIQ_STATUS_T
+retrieve_message(VCHIQ_SERVICE_HANDLE_T service, VCHIU_QUEUE_T *queue)
+{
+   VCHIQ_HEADER_T *queued_header, *header;
+   uint32_t size;
+
+   if (swi_vchiq_msg_peek(service, 0, (void **)&queued_header, &size))
+      return VCHIQ_ERROR;
+
+   /* Make a copy of the message that we can place in our own queue.  */
+   header = malloc(size + sizeof(VCHIQ_HEADER_T));
+   if (!header)
+      return VCHIQ_ERROR;
+
+   /* We're given a pointer to the data in the message, so we have to 
+    * step back to find the message header.  */
+   queued_header--;
+
+   memcpy(header, queued_header, sizeof(VCHIQ_HEADER_T) + queued_header->size);
+   vchi_msg_remove(service);
+   vchiu_queue_push(queue, header);
+
+   return VCHIQ_SUCCESS;
+}
+
+VCHIQ_STATUS_T khrn_callback(void *callback_param,
+			     VCHI_CALLBACK_REASON_T reason,
+			     void *bulk_userdata )
 {
    switch (reason) {
-   case VCHIQ_MESSAGE_AVAILABLE:
-      vchiu_queue_push(&khrn_queue, header);
-      break;
-   case VCHIQ_BULK_TRANSMIT_DONE:
-   case VCHIQ_BULK_RECEIVE_DONE:
+   case VCHI_CALLBACK_MSG_AVAILABLE:
+      return retrieve_message(vchiq_khrn_service, &khrn_queue);
+   case VCHI_CALLBACK_BULK_SENT:
       vcos_event_signal(&bulk_event);
       break;
-   case VCHIQ_SERVICE_OPENED:
-   case VCHIQ_SERVICE_CLOSED:
-   case VCHIQ_BULK_TRANSMIT_ABORTED:
-   case VCHIQ_BULK_RECEIVE_ABORTED:
+   case VCHI_CALLBACK_SERVICE_CLOSED:
+      break;
+   case VCHI_CALLBACK_BULK_TRANSMIT_ABORTED:
+   case VCHI_CALLBACK_BULK_RECEIVE_ABORTED:
       UNREACHABLE(); /* not implemented */
    }
 
    return VCHIQ_SUCCESS;
 }
 
-VCHIQ_STATUS_T khhn_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
-                  VCHIQ_SERVICE_HANDLE_T handle, void *bulk_userdata)
+VCHIQ_STATUS_T khhn_callback(void *callback_param,
+			     VCHI_CALLBACK_REASON_T reason,
+			     void *bulk_userdata )
 {
    switch (reason) {
-   case VCHIQ_MESSAGE_AVAILABLE:
-      vchiu_queue_push(&khhn_queue, header);
-      break;
-   case VCHIQ_BULK_TRANSMIT_DONE:
-   case VCHIQ_BULK_RECEIVE_DONE:
+   case VCHI_CALLBACK_MSG_AVAILABLE:
+      return retrieve_message(vchiq_khhn_service, &khhn_queue);
+   case VCHI_CALLBACK_BULK_SENT:
       vcos_event_signal(&bulk_event);
       break;
-   case VCHIQ_SERVICE_OPENED:
-   case VCHIQ_SERVICE_CLOSED:
-   case VCHIQ_BULK_TRANSMIT_ABORTED:
-   case VCHIQ_BULK_RECEIVE_ABORTED:
-      UNREACHABLE(); /* not implemented */      
+   case VCHI_CALLBACK_SERVICE_CLOSED:
+      break;
+   case VCHI_CALLBACK_BULK_TRANSMIT_ABORTED:
+   case VCHI_CALLBACK_BULK_RECEIVE_ABORTED:
+      UNREACHABLE(); /* not implemented */
    }
 
    return VCHIQ_SUCCESS;
 }
 
+VCHIQ_STATUS_T khan_callback(void *callback_param,
+			     VCHI_CALLBACK_REASON_T reason,
+			     void *bulk_userdata )
+{
+   switch (reason) {
+   case VCHI_CALLBACK_SERVICE_CLOSED:
+   case VCHI_CALLBACK_SERVICE_OPENED:
+      break;
+   default:
+      report_text(__func__);
+      report_dec(reason);
+      break;
+   }
+
+   return VCHIQ_SUCCESS;
+}
+#if 0
 VCHIQ_STATUS_T khan_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T *header,
                   VCHIQ_SERVICE_HANDLE_T handle, void *bulk_userdata)
 {
@@ -165,7 +208,7 @@
 
    return VCHIQ_SUCCESS;
 }
-
+#endif
 void vc_vchi_khronos_init()
 {
    VCOS_STATUS_T status = vcos_event_create(&bulk_event, NULL);
@@ -196,15 +239,15 @@
    params.version_min = VC_KHRN_VERSION;
 
    params.fourcc = FOURCC_KHAN;
-   params.callback = khan_callback;
+   params.callback = asm_khan_callback;
    khan_return = vchiq_open_service(khrn_vchiq_instance, &params, &vchiq_khan_service);
 
    params.fourcc = FOURCC_KHRN;
-   params.callback = khrn_callback;
+   params.callback = asm_khrn_callback;
    khrn_return = vchiq_open_service(khrn_vchiq_instance, &params, &vchiq_khrn_service);
 
    params.fourcc = FOURCC_KHHN;
-   params.callback = khhn_callback;
+   params.callback = asm_khhn_callback;
    khhn_return = vchiq_open_service(khrn_vchiq_instance, &params, &vchiq_khhn_service);
 
    if (khan_return != VCHIQ_SUCCESS ||