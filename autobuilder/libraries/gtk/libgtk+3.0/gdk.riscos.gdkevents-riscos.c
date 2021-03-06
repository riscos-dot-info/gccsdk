/* GDK - The GIMP Drawing Kit
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Written by Lee Noar using code and guidance from other targets.
 */

#include "gdkdisplay-riscos.h"
#include "gdkriscos.h"
#include "gdkscreen-riscos.h"
#include "gdkriscoswindow.h"
#include "gdkwindow-riscos.h"
#include "gdkprivate-riscos.h"
#include "gdkdevicemanager-riscos.h"

#include "oslib/wimp.h"
#include "oslib/osspriteop.h"
#include "oslib/os.h"
#include "oslib/report.h"
#include "oslib/osbyte.h"

#include "cairo-riscos.h"

//#define G_MAINLOOP_POLL
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <swis.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define DEBUG_REDRAW 0

typedef struct _raw_event_handler_data
{
  wimp_event_no type;
  wimp_w window_handle;
  gdk_riscos_raw_event_handler handler_func;
  void *handler_data;

} raw_event_handler_data;

typedef struct _message_handler_data
{
  unsigned message_no;
  gdk_riscos_message_handler handler_func;
  void *data;

} message_handler_data;

typedef struct _GdkEventSource
{
  GSource source;

  GdkDisplay *display;
} GdkEventSource;

/* TODO: Move keyboard stuff to gdkkeys-riscos.c */
struct keymapping
{
    int riscos_key_code;
    guint gdk_key_code;
};

struct internal_mapping
{
  guint wimp_code;
  guint internal_code;
};

/* A hashmap may be better at runtime, but still needs to be initialised from
 * something like the following.
 * TODO: check some of these codes at runtime.
 */
static const struct internal_mapping internal_keymap[] = {
  { 8, 47 },
  { 127, 89 },
  { ' ', 98 },
  { '!', 48 },
  { '"', 49 },
  { '#', 45 },
  { '$', 18 },
  { '%', 19 },
  { '^', 24 },
  { '&', 36 },
  { '\'', 79 },
  { '(', 38 },
  { ')', 39 },
  { '*', 21 },
  { ',', 102 },
  { '-', 23 },
  { '.', 104 },
  { '/', 39 },
  { '0', 48 },
  { '1', 49 },
  { '2', 17 },
  { '3', 18 },
  { '4', 19 },
  { '5', 52 },
  { '6', 36 },
  { '7', 21 },
  { '8', 38 },
  { '9', 39 },
  { ':', 87 },
  { ';', 87 },
  { '<', 102 },
  { '=', 93 },
  { '>', 103 },
  { '?', 104 },
  { '@', 79 },
  { 'A', 65 },
  { 'B', 100 },
  { 'C', 82 },
  { 'D', 50 },
  { 'E', 34 },
  { 'F', 67 },
  { 'G', 83 },
  { 'H', 84 },
  { 'I', 37 },
  { 'J', 69 },
  { 'K', 70 },
  { 'L', 86 },
  { 'M', 101 },
  { 'N', 85 },
  { 'O', 54 },
  { 'P', 55 },
  { 'Q', 16 },
  { 'R', 51 },
  { 'S', 81 },
  { 'T', 35 },
  { 'U', 53 },
  { 'V', 99 },
  { 'W', 33 },
  { 'X', 66 },
  { 'Y', 68 },
  { 'Z', 97 },
  { '[', 56 },
  { ']', 88 },
  { '\\', 120 },
  { '_', 23 },
  { '`', 45 },
  { 27, 112 },
  { 'a', 65 },
  { 'b', 100 },
  { 'c', 82 },
  { 'd', 50 },
  { 'e', 34 },
  { 'f', 67 },
  { 'g', 83 },
  { 'h', 84 },
  { 'i', 37 },
  { 'j', 69 },
  { 'k', 70 },
  { 'l', 86 },
  { 'm', 101 },
  { 'n', 85 },
  { 'o', 54 },
  { 'p', 55 },
  { 'q', 16 },
  { 'r', 51 },
  { 's', 81 },
  { 't', 35 },
  { 'u', 53 },
  { 'v', 99 },
  { 'w', 33 },
  { 'x', 66 },
  { 'y', 68 },
  { 'z', 97 },
  { '{', 56 },
  { '}', 88 },
  { '|', 94 },
  { '~', 120 }
};

static const struct keymapping function_keymap[] = {
    { wimp_KEY_LOGO, GDK_KEY_Meta_L },
    { wimp_KEY_MENU, GDK_KEY_Menu },
    { wimp_KEY_BACKSPACE, GDK_KEY_BackSpace },
    { wimp_KEY_RETURN, GDK_KEY_Return },
    { wimp_KEY_ESCAPE, GDK_KEY_Escape },
    { wimp_KEY_HOME, GDK_KEY_Home },
    { wimp_KEY_DELETE, GDK_KEY_Delete },
    { wimp_KEY_PRINT, GDK_KEY_Print },
    { wimp_KEY_F1, GDK_KEY_F1 },
    { wimp_KEY_F2, GDK_KEY_F2 },
    { wimp_KEY_F3, GDK_KEY_F3 },
    { wimp_KEY_F4, GDK_KEY_F4 },
    { wimp_KEY_F5, GDK_KEY_F5 },
    { wimp_KEY_F6, GDK_KEY_F6 },
    { wimp_KEY_F7, GDK_KEY_F7 },
    { wimp_KEY_F8, GDK_KEY_F8 },
    { wimp_KEY_F9, GDK_KEY_F9 },
    { wimp_KEY_TAB, GDK_KEY_Tab },
    { wimp_KEY_COPY, GDK_KEY_End },
    { wimp_KEY_LEFT, GDK_KEY_Left },
    { wimp_KEY_RIGHT, GDK_KEY_Right },
    { wimp_KEY_DOWN, GDK_KEY_Down },
    { wimp_KEY_UP, GDK_KEY_Up },
    { wimp_KEY_F10, GDK_KEY_F10 },
    { wimp_KEY_F11, GDK_KEY_F11 },
    { wimp_KEY_F12, GDK_KEY_F12 },
    { wimp_KEY_INSERT, GDK_KEY_Insert },
    { wimp_KEY_PRINT | wimp_KEY_SHIFT, GDK_KEY_Print },
    { wimp_KEY_F1 | wimp_KEY_SHIFT, GDK_KEY_F1 },
    { wimp_KEY_F2 | wimp_KEY_SHIFT, GDK_KEY_F2 },
    { wimp_KEY_F3 | wimp_KEY_SHIFT, GDK_KEY_F3 },
    { wimp_KEY_F4 | wimp_KEY_SHIFT, GDK_KEY_F4 },
    { wimp_KEY_F5 | wimp_KEY_SHIFT, GDK_KEY_F5 },
    { wimp_KEY_F6 | wimp_KEY_SHIFT, GDK_KEY_F6 },
    { wimp_KEY_F7 | wimp_KEY_SHIFT, GDK_KEY_F7 },
    { wimp_KEY_F8 | wimp_KEY_SHIFT, GDK_KEY_F8 },
    { wimp_KEY_F9 | wimp_KEY_SHIFT, GDK_KEY_F9 },
    { wimp_KEY_TAB | wimp_KEY_SHIFT, GDK_KEY_Tab },
    { wimp_KEY_COPY | wimp_KEY_SHIFT, GDK_KEY_End },
    { wimp_KEY_LEFT | wimp_KEY_SHIFT, GDK_KEY_Left },
    { wimp_KEY_RIGHT | wimp_KEY_SHIFT, GDK_KEY_Right },
    { wimp_KEY_DOWN | wimp_KEY_SHIFT, GDK_KEY_Page_Down },
    { wimp_KEY_UP | wimp_KEY_SHIFT, GDK_KEY_Page_Up },
    { wimp_KEY_F10 | wimp_KEY_SHIFT, GDK_KEY_F10 },
    { wimp_KEY_F11 | wimp_KEY_SHIFT, GDK_KEY_F11 },
    { wimp_KEY_F12 | wimp_KEY_SHIFT, GDK_KEY_F12 },
    { wimp_KEY_INSERT | wimp_KEY_SHIFT, GDK_KEY_Insert },
    { wimp_KEY_PRINT | wimp_KEY_CONTROL, GDK_KEY_Print },
    { wimp_KEY_F1 | wimp_KEY_CONTROL, GDK_KEY_F1 },
    { wimp_KEY_F2 | wimp_KEY_CONTROL, GDK_KEY_F2 },
    { wimp_KEY_F3 | wimp_KEY_CONTROL, GDK_KEY_F3 },
    { wimp_KEY_F4 | wimp_KEY_CONTROL, GDK_KEY_F4 },
    { wimp_KEY_F5 | wimp_KEY_CONTROL, GDK_KEY_F5 },
    { wimp_KEY_F6 | wimp_KEY_CONTROL, GDK_KEY_F6 },
    { wimp_KEY_F7 | wimp_KEY_CONTROL, GDK_KEY_F7 },
    { wimp_KEY_F8 | wimp_KEY_CONTROL, GDK_KEY_F8 },
    { wimp_KEY_F9 | wimp_KEY_CONTROL, GDK_KEY_F9 },
    { wimp_KEY_TAB | wimp_KEY_CONTROL, GDK_KEY_Tab },
    { wimp_KEY_COPY | wimp_KEY_CONTROL, GDK_KEY_End },
    { wimp_KEY_LEFT | wimp_KEY_CONTROL, GDK_KEY_Left },
    { wimp_KEY_RIGHT | wimp_KEY_CONTROL, GDK_KEY_Right },
    { wimp_KEY_DOWN | wimp_KEY_CONTROL, GDK_KEY_Down },
    { wimp_KEY_UP | wimp_KEY_CONTROL, GDK_KEY_Up },
    { wimp_KEY_F10 | wimp_KEY_CONTROL, GDK_KEY_F10 },
    { wimp_KEY_F11 | wimp_KEY_CONTROL, GDK_KEY_F11 },
    { wimp_KEY_F12 | wimp_KEY_CONTROL, GDK_KEY_F12 },
    { wimp_KEY_INSERT | wimp_KEY_CONTROL, GDK_KEY_Insert },
    { wimp_KEY_PRINT | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_Print },
    { wimp_KEY_F1 | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_F1 },
    { wimp_KEY_F2 | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_F2 },
    { wimp_KEY_F3 | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_F3 },
    { wimp_KEY_F4 | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_F4 },
    { wimp_KEY_F5 | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_F5 },
    { wimp_KEY_F6 | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_F6 },
    { wimp_KEY_F7 | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_F7 },
    { wimp_KEY_F8 | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_F8 },
    { wimp_KEY_F9 | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_F9 },
    { wimp_KEY_TAB | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_Tab },
    { wimp_KEY_COPY | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_End },
    { wimp_KEY_LEFT | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_Left },
    { wimp_KEY_RIGHT | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_Right },
    { wimp_KEY_DOWN | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_Down },
    { wimp_KEY_UP | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_Up },
    { wimp_KEY_F10 | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_F10 },
    { wimp_KEY_F11 | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_F11 },
    { wimp_KEY_F12 | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_F12 },
    { wimp_KEY_INSERT | wimp_KEY_SHIFT | wimp_KEY_CONTROL, GDK_KEY_Insert }
};

extern void __pthread_disable_ints(void);
extern void __pthread_enable_ints(void);

void
gdk_riscos_add_raw_event_handler (wimp_event_no type,
				  wimp_w window_handle,
				  gdk_riscos_raw_event_handler handler_func,
				  void *handler_data)
{
  GdkRiscosDisplay *riscos_display = GDK_RISCOS_DISPLAY(_gdk_display);
  raw_event_handler_data *event;

  event = g_new (raw_event_handler_data, 1);
  event->type = type;
  event->window_handle = window_handle;
  event->handler_func = handler_func;
  event->handler_data = handler_data;

  riscos_display->raw_event_handlers = g_list_append (riscos_display->raw_event_handlers,
						      event);
}

void
gdk_riscos_remove_raw_event_handler (wimp_event_no type,
				     wimp_w window_handle,
				     gdk_riscos_raw_event_handler handler_func,
				     void *handler_data)
{
  GdkRiscosDisplay *riscos_display = GDK_RISCOS_DISPLAY(_gdk_display);
  GList *list = g_list_first (riscos_display->raw_event_handlers);
  GList *node = g_list_first(list);

  while (node)
    {
      GList *next = g_list_next(node);
      raw_event_handler_data *handler = node->data;
      if (handler->type == type &&
	  handler->window_handle == window_handle &&
	  handler->handler_func == handler_func &&
	  handler->handler_data == handler_data) {
	g_free(handler);
        list = g_list_delete_link(list, node);
      }
      node = next;
    }

  riscos_display->raw_event_handlers = list;
}

void
gdk_riscos_add_message_handler (unsigned message_no,
				gdk_riscos_message_handler handler_func,
				void *data)
{
  GdkRiscosDisplay *riscos_display = GDK_RISCOS_DISPLAY(_gdk_display);
  message_handler_data *message;

  message = g_new (message_handler_data, 1);
  message->message_no = message_no;
  message->handler_func = handler_func;
  message->data = data;

  riscos_display->message_handlers = g_list_append (riscos_display->message_handlers,
						    message);
}

void
gdk_riscos_remove_message_handler (unsigned message_no,
				   gdk_riscos_message_handler handler_func,
				   void *data)
{
  GdkRiscosDisplay *riscos_display = GDK_RISCOS_DISPLAY(_gdk_display);
  GList *list = riscos_display->message_handlers;
  GList *node = g_list_first(list);

  while (node)
    {
      GList *next = g_list_next(node);
      message_handler_data *handler = node->data;
      if (handler->message_no == message_no &&
	  handler->handler_func == handler_func &&
	  handler->data == data) {
	g_free(handler);
        list = g_list_delete_link(list, node);
      }
      node = next;
    }

  riscos_display->message_handlers = list;
}

static wimp_w
get_event_window (wimp_block *poll_block, wimp_event_no event_type)
{
  wimp_w result = wimp_BACKGROUND;

  switch (event_type)
    {
    case wimp_REDRAW_WINDOW_REQUEST:
      result = poll_block->redraw.w;
      break;
    case wimp_OPEN_WINDOW_REQUEST:
      result = poll_block->open.w;
      break;
    case wimp_CLOSE_WINDOW_REQUEST:
      result = poll_block->close.w;
      break;
    case wimp_POINTER_ENTERING_WINDOW:
    case wimp_POINTER_LEAVING_WINDOW:
      result = poll_block->entering.w;
      break;
    case wimp_MOUSE_CLICK:
      result = poll_block->pointer.w;
      break;
    case wimp_KEY_PRESSED:
      result = poll_block->key.w;
      break;
    }

  return result;
}

static gboolean
call_raw_event_handlers (GdkRiscosDisplay *riscos_display,
			 wimp_block *poll_block,
			 wimp_event_no event_type)
{
  GList *list = g_list_first (riscos_display->raw_event_handlers);
  wimp_w event_window = get_event_window (poll_block, event_type);
  while (list)
    {
      raw_event_handler_data *raw_event = list->data;
      if ((raw_event->type == (wimp_event_no)-1 || event_type == raw_event->type) &&
	  (raw_event->window_handle == wimp_BACKGROUND || event_window == raw_event->window_handle))
	if (raw_event->handler_func (event_type, poll_block, raw_event->handler_data))
	  return TRUE;

      list = g_list_next(list);
    }

  return FALSE;
}

static gboolean
call_message_handlers (int event_no,
		       GdkRiscosDisplay *riscos_display,
		       wimp_block *poll_block)
{
  GList *list = g_list_first (riscos_display->message_handlers);
  wimp_message *message = &poll_block->message;

  while (list)
    {
      message_handler_data *handler = list->data;
      if (message->action == handler->message_no)
	if (handler->handler_func (event_no, poll_block, handler->data))
	  return TRUE;
      list = g_list_next(list);
    }

  return FALSE;
}

static void
send_event (GdkEvent *event)
{
  GdkDisplay *display = gdk_window_get_display (event->any.window);
  GList *node = _gdk_event_queue_append (display, event);

  _gdk_windowing_got_event (display,
			    node,
			    event,
			    _gdk_display_get_next_serial (display));
}

static os_error *
null_redraw (wimp_draw *redraw)
{
    os_error *err;
    osbool more;

#if DEBUG_REDRAW
    g_warning ("RISCOS: Unrecognised window %p during redraw event.\n", redraw->w);
#endif

    __pthread_disable_ints();

    err = xwimp_redraw_window (redraw, &more);
    if (!err) {
	while (more)
	    err = xwimp_get_rectangle (redraw, &more);
    }

    __pthread_enable_ints();

    return err;
}

static void
gdk_riscos_handle_redraw_event (GdkRiscosDisplay *riscos_display,
				wimp_block *poll_block)
{
  wimp_draw *redraw = &poll_block->redraw;
  GdkWindow *window;

  window = g_hash_table_lookup (riscos_display->id_ht,
			        redraw->w);
  if (!window)
    {
      null_redraw(redraw);
      return;
    }

  GdkWindowImplRiscos *impl = GDK_WINDOW_IMPL_RISCOS (window->impl);
  os_coord origin;
#if DEBUG_REDRAW
  os_error *err = NULL;
#endif
  osbool more;

  __pthread_disable_ints();

  xwimp_redraw_window (redraw, &more);

  origin.x = redraw->box.x0 - redraw->xscroll;
  origin.y = redraw->box.y1 - redraw->yscroll - gdk_riscos_window_os_height (window);

  while (more)
    {
#if DEBUG_REDRAW
      err =
#endif
	xosspriteop_put_sprite_user_coords (osspriteop_PTR,
					    impl->surface_sprite_area,
					    (osspriteop_id)impl->surface_sprite_pointer,
					    origin.x,
					    origin.y,
					    os_ACTION_OVERWRITE);
      xwimp_get_rectangle (redraw, &more);
    }

  __pthread_enable_ints();

#if DEBUG_REDRAW
  if (err)
    g_warning ("RISC OS: Failed to redraw Cairo surface; %s", err->errmess);
#endif
}

static void
gdk_riscos_handle_open_event (GdkRiscosDisplay *riscos_display,
			      wimp_block *poll_block)
{
  wimp_open *open = &poll_block->open;
  GdkWindow *gwindow;

  gwindow = g_hash_table_lookup (riscos_display->id_ht,
			         open->w);
  if (gwindow == NULL)
    return;

  GdkWindowImplRiscos *impl = GDK_WINDOW_IMPL_RISCOS (gwindow->impl);

  GdkPoint old_pos;
  gdk_window_get_position (gwindow, &old_pos.x, &old_pos.y);

  GdkPoint new_os_size;
  new_os_size.x = open->visible.x1 - open->visible.x0;
  new_os_size.y = open->visible.y1 - open->visible.y0;
  
  if (new_os_size.x < impl->min_os_size.x || new_os_size.y < impl->min_os_size.y)
    return;

  xwimp_open_window (open);

  GdkPoint old_size;
  old_size.x = gdk_window_get_width (gwindow);
  old_size.y = gdk_window_get_height (gwindow);

  GdkPoint new_size;
  gdk_riscos_screen_point_to_pixel (impl->screen, &new_os_size, &new_size);

  GdkPoint new_pos;
  new_pos.x = open->visible.x0;
  new_pos.y = gdk_riscos_screen_os_height (impl->screen) - open->visible.y1;
  gdk_riscos_screen_point_to_pixel (impl->screen, &new_pos, &new_pos);

  if (old_size.x == new_size.x && old_size.y == new_size.y &&
      old_pos.x == new_pos.x && old_pos.y == new_pos.y)
    return;

  if (new_size.x != old_size.x || new_size.y != old_size.y)
    {
      gwindow->width = new_size.x;
      gwindow->height = new_size.y;
      _gdk_riscos_window_resize_surface (gwindow);
    }

  gwindow->x = new_pos.x;
  gwindow->y = new_pos.y;

  _gdk_window_update_size (gwindow);
  
  GdkEvent *gdk_event;

  gdk_event = gdk_event_new (GDK_CONFIGURE);
  gdk_event->configure.window = g_object_ref (gwindow); 
  gdk_event->configure.send_event = FALSE;
  gdk_event->configure.x = new_pos.x;
  gdk_event->configure.y = new_pos.y;
  gdk_event->configure.width = new_size.x;
  gdk_event->configure.height = new_size.y;

  send_event (gdk_event);
}

/* We don't actually close the window here, we merely report it. It's upto
 * the caller whether the window should be closed or some other action taken.  */
static void
gdk_riscos_handle_close_event (GdkRiscosDisplay *riscos_display,
			       wimp_block *poll_block)
{
  wimp_close *riscos_event = &poll_block->close;
  GdkWindow *window;
  GdkEvent *gdk_event;

  window = g_hash_table_lookup (riscos_display->id_ht,
			        riscos_event->w);
  if (window == NULL)
    return;

  gdk_event = gdk_event_new (GDK_DELETE);
  gdk_event->any.window = g_object_ref (window);
  gdk_event->any.send_event = FALSE;

  send_event (gdk_event);
}

static GdkEvent *
create_focus_event (GdkRiscosDisplay *rdisplay,
		    GdkWindow *window,
		    gboolean   in)
{
  GdkEvent *event;
  GdkRISCOSDeviceManager *device_manager;

  event = gdk_event_new (GDK_FOCUS_CHANGE);
  event->focus_change.window = g_object_ref (window);
  event->focus_change.in = in;

  device_manager = GDK_RISCOS_DEVICE_MANAGER (rdisplay->parent_instance.device_manager);
  gdk_event_set_device (event, device_manager->core_keyboard);

  return event;
}

static void
_gdk_riscos_update_focus_window (GdkRiscosDisplay *rdisplay,
				 GdkWindow *window,
				 gboolean got_focus)
{
  GdkEvent *event;

  /* Nothing to do if we already have the input focus.  */
  if (got_focus && window == rdisplay->focus_window)
    return;

  if (!got_focus && window == rdisplay->focus_window)
    {
      /* We just lost the input focus.  */
      event = create_focus_event (rdisplay, rdisplay->focus_window, FALSE);
      send_event (event);
      g_object_unref (rdisplay->focus_window);
      rdisplay->focus_window = NULL;
    }

  if (got_focus)
    {
      /* We gained the input focus...  */
      if (rdisplay->focus_window)
	{
	  /* ...and caused another GDK window to lose it.  */
	  event = create_focus_event (rdisplay, rdisplay->focus_window, FALSE);
	  send_event (event);
	  g_object_unref (rdisplay->focus_window);
	  rdisplay->focus_window = NULL;
	}

      event = create_focus_event (rdisplay, window, TRUE);
      send_event (event);
      rdisplay->focus_window = g_object_ref (window);

      /* Broadcast the fact that we are claiming the clipboard/selection/caret.  */
      rdisplay->poll_block.message.size = sizeof(wimp_full_message_claim_entity);
      rdisplay->poll_block.message.your_ref = 0;
      rdisplay->poll_block.message.action = message_CLAIM_ENTITY;
      rdisplay->poll_block.message.data.claim_entity.flags = wimp_CLAIM_CARET_OR_SELECTION;
      xwimp_send_message(wimp_USER_MESSAGE, &rdisplay->poll_block.message, wimp_BROADCAST);
    }
}

static void
gdk_riscos_handle_message (int event_no,
			   GdkRiscosDisplay *riscos_display,
			   wimp_block *poll_block)
{
  wimp_message *message = &poll_block->message;

  if (call_message_handlers (event_no, riscos_display, poll_block))
    return;

  if (event_no == wimp_USER_MESSAGE_ACKNOWLEDGE)
    {
      switch (message->action)
	{
	case message_RAM_TRANSMIT:
	  if (riscos_display->selection_xfer_progress)
	    {
	      free(riscos_display->selection_xfer_progress);
	      riscos_display->selection_xfer_progress = NULL;
	    }
	  break;
	}
      return;
    }

  switch (message->action)
    {
    case message_DATA_LOAD:
      if (message->my_ref == 0) /* 0 = from the filer */
	{
	  GdkWindow *gdk_window = g_hash_table_lookup (riscos_display->id_ht,
						       message->data.data_xfer.w);
	  if (gdk_window)
	    {
#if 0
	      _gdk_riscos_display_set_last_seen_time (rdisplay);

	      GdkEvent *event = gdk_event_new (GDK_KEY_PRESS);
	      event->key.window = g_object_ref (gdk_window);
	      event->key.send_event = FALSE;
	      event->key.time = riscos_display->last_seen_time;
	      event->key.state = GDK_CONTROL_MASK;
	      event->key.keyval = map_keyboard_wimp_to_gdk ('V', GDK_CONTROL_MASK);
	      event->key.length = 0;
	      event->key.string = NULL;
	      event->key.hardware_keycode = event->key.keyval;
	      event->key.group = 0;
	      event->key.is_modifier = FALSE;

	      gdk_event_set_device (event, riscos_display->parent_instance.core_pointer);

	      send_event (event);
#endif
	    }
	}
      break;

    case message_CLAIM_ENTITY: {
      if (message->sender != riscos_display->task_handle &&
	  message->data.claim_entity.flags & wimp_CLAIM_CARET_OR_SELECTION &&
	  riscos_display->focus_window)
	_gdk_riscos_update_focus_window(riscos_display, riscos_display->focus_window, FALSE);
      break;
    }

    case message_RELEASE_ENTITY:
/*      printf("message_RELEASE_ENTITY");*/
      break;

    case message_DATA_REQUEST: {
      if ((message->data.data_request.flags & wimp_DATA_REQUEST_CLIPBOARD) == 0 ||
	  !riscos_display->selection_window)
	break;

      message->size = sizeof(wimp_full_message_data_xfer);
      message->your_ref = message->my_ref;
      message->action = message_DATA_SAVE;
      message->data.data_xfer.est_size = riscos_display->selection.size;
      message->data.data_xfer.file_type = 0xFFF;
      strcpy(message->data.data_xfer.file_name, "gtk-selection");
      xwimp_send_message(wimp_USER_MESSAGE_RECORDED, message, message->sender);

      riscos_display->selection_xfer_progress = malloc(sizeof(GdkRiscosSelectionXferProgress));
      riscos_display->selection_xfer_progress->bytes_remaining = riscos_display->selection.size;
      riscos_display->selection_xfer_progress->bytes_sent = 0;
      
      break;
    }

    case message_RAM_FETCH: {
      if (!riscos_display->selection_window ||
	  !riscos_display->selection.buffer ||
	  !riscos_display->selection_xfer_progress)
	break;

      /* Send as much data as the receiver can handle.  */
      size_t bytes_to_transfer = MIN(message->data.ram_xfer.size, riscos_display->selection_xfer_progress->bytes_remaining);
      if (bytes_to_transfer > 0)
	{
	  xwimp_transfer_block(riscos_display->task_handle,
			       riscos_display->selection.buffer + riscos_display->selection_xfer_progress->bytes_sent,
			       message->sender,
			       message->data.ram_xfer.addr,
			       bytes_to_transfer);
	  riscos_display->selection_xfer_progress->bytes_remaining -= bytes_to_transfer;
	  riscos_display->selection_xfer_progress->bytes_sent += bytes_to_transfer;
	}

      message->size = sizeof(wimp_full_message_ram_xfer);
      message->your_ref = message->my_ref;
      message->action = message_RAM_TRANSMIT;
      message->data.ram_xfer.size = bytes_to_transfer;
      /* Only send recorded if there is more data to send.  */
      xwimp_send_message(riscos_display->selection_xfer_progress->bytes_remaining > 0 ?
			    wimp_USER_MESSAGE_RECORDED :
			    wimp_USER_MESSAGE,
			 message,
			 message->sender);
      if (riscos_display->selection_xfer_progress->bytes_remaining == 0)
	{
	  free(riscos_display->selection_xfer_progress);
	  riscos_display->selection_xfer_progress = NULL;
	}

      break;
    }

    case message_QUIT:
      exit (0);
      break;
    }
}

static guint
translate_mouse_button (wimp_mouse_state mouse_button)
{
  guint state = 0;

  if (mouse_button & (wimp_DOUBLE_SELECT | wimp_SINGLE_SELECT | wimp_DRAG_SELECT))
    state = GDK_BUTTON_PRIMARY;
  else if (mouse_button & wimp_CLICK_MENU)
    state = GDK_BUTTON_MIDDLE;
  else if (mouse_button & (wimp_DOUBLE_ADJUST | wimp_SINGLE_ADJUST | wimp_DRAG_ADJUST))
    state = GDK_BUTTON_SECONDARY;

  return state;
}

/* Read state of mouse buttons.  */
guint
gdk_riscos_translate_buttons (guint mouse_buttons)
{
  guint state = 0;

  if (mouse_buttons & (wimp_DOUBLE_SELECT | wimp_SINGLE_SELECT | wimp_DRAG_SELECT))
    state |= GDK_BUTTON1_MASK;
  if (mouse_buttons & wimp_CLICK_MENU)
    state |= GDK_BUTTON2_MASK;
  if (mouse_buttons & (wimp_DOUBLE_ADJUST | wimp_SINGLE_ADJUST | wimp_DRAG_ADJUST))
    state |= GDK_BUTTON3_MASK;

  return state;
}

/* Read state of keyboard modifier keys.  */
guint
gdk_riscos_read_modifiers (void)
{
  guint state = 0;

  /* Check for shift key. GDK doesn't seem to differentiate between left and right.
   * Use internal key code of 0 which covers both.  */
  if (osbyte1 (osbyte_IN_KEY, 0 ^ 0xff, 0xff))
    state |= GDK_SHIFT_MASK;

  /* Check for control key. Internal key code 1 covers both left and right.  */
  if (osbyte1 (osbyte_IN_KEY, 1 ^ 0xff, 0xff))
    state |= GDK_CONTROL_MASK;

  /* Check for alt key. Internal key code 2 covers both left and right.
   * According to:
   * 
   * https://developer.gnome.org/gdk/stable/gdk-Windows.html#GdkModifierType
   * 
   * the mapping of GDK_MOD1_MASK depends on how the server is configured, but
   * it's usually the Alt key.  */
  if (osbyte1 (osbyte_IN_KEY, 2 ^ 0xff, 0xff))
    state |= GDK_MOD1_MASK;

  /* Check the caps lock key using internal key code of 64.  */
  if (osbyte1 (osbyte_IN_KEY, 64 ^ 0xff, 0xff))
    state |= GDK_LOCK_MASK;

  return state;
}

static void
dump_pointer (wimp_pointer *pointer)
{
  printf ("****\n"
	  "Pointer:\n"
	  " Window: %p\n"
	  " x, y: %d, %d\n"
	  " buttons: %x\n"
	  "****\n",
	  pointer->w,
	  pointer->pos.x,
	  pointer->pos.y,
	  pointer->buttons);
}

static GdkEvent *
create_button_event (GdkRiscosDisplay *rdisplay,
		     GdkWindow *gdk_window,
		     GdkEventType type)
{
  GdkRISCOSDeviceManager *device_manager =
    (GdkRISCOSDeviceManager *)gdk_display_get_device_manager (gdk_window_get_display (gdk_window));

  GdkEvent *event = gdk_event_new (type);

  event->button.window = g_object_ref (gdk_window);
  event->button.send_event = FALSE;
  event->button.time = rdisplay->last_seen_time;
  event->button.axes = NULL;
  event->button.device = device_manager->core_pointer;

  return event;
}

static void
start_resize_window (GdkRiscosDisplay *rdisplay,
		     wimp_pointer *mouse)
{
  wimp_drag drag;

  drag.w = mouse->w;
  drag.type = wimp_DRAG_SYSTEM_SIZE;
  if (xwimp_drag_box(&drag) == NULL)
    {
      rdisplay->system_drag = TRUE;
    }
}

/*static void
gdk_riscos_handle_gain_focus_event (GdkRiscosDisplay *rdisplay,
				    wimp_block *poll_block)
{
  wimp_caret *riscos_event = &poll_block->caret;
  GdkWindow *gdk_window;

  gdk_window = g_hash_table_lookup (rdisplay->id_ht,
				    riscos_event->w);
  if (gdk_window == NULL)
    return;

  _gdk_riscos_update_focus_window (rdisplay, gdk_window, TRUE);
}

static void
gdk_riscos_handle_lose_focus_event (GdkRiscosDisplay *rdisplay,
				    wimp_block *poll_block)
{
  wimp_caret *riscos_event = &poll_block->caret;
  GdkWindow *gdk_window;

  gdk_window = g_hash_table_lookup (rdisplay->id_ht,
				    riscos_event->w);
  if (gdk_window == NULL)
    return;

  _gdk_riscos_update_focus_window (rdisplay, gdk_window, FALSE);
}*/

static void
gdk_riscos_handle_mouse_click (GdkRiscosDisplay *rdisplay,
			       wimp_block *poll_block)
{
  wimp_pointer *mouse = &poll_block->pointer;

  GdkWindow *gdk_window = g_hash_table_lookup (rdisplay->id_ht, mouse->w);
  if (!gdk_window)
    return;

  GdkWindowImplRiscos *impl = GDK_WINDOW_IMPL_RISCOS (gdk_window->impl);

  GdkPoint mouse_pos, global_pos, local_pos;
  gdk_riscos_screen_point_to_pixel (impl->screen, (GdkPoint *)&mouse->pos, &mouse_pos);
  gdk_riscos_screen_global_point (impl->screen, &mouse_pos, &global_pos);
  gdk_riscos_window_map_from_global (impl->wrapper, &mouse_pos, &local_pos);

  guint modifiers = gdk_riscos_read_modifiers ();
  guint buttons = gdk_riscos_translate_buttons (mouse->buttons);

  gdk_riscos_window_set_caret (gdk_window);

#if 0
  if (buttons & GDK_BUTTON1_MASK && modifiers & GDK_CONTROL_MASK)
    gdk_riscos_window_dump_to_file (gdk_window, "mem:$.surface");
#endif

  GdkEvent *event = create_button_event (rdisplay, gdk_window, GDK_BUTTON_PRESS);
  event->button.x = local_pos.x;
  event->button.y = local_pos.y;
  event->button.state = modifiers | buttons;
  event->button.button = translate_mouse_button (mouse->buttons);
  event->button.x_root = global_pos.x;
  event->button.y_root = global_pos.y;

  send_event (event);

  rdisplay->last_click = *mouse;

  _gdk_riscos_update_focus_window (rdisplay, gdk_window, TRUE);
}

static void
gdk_riscos_handle_enter_event (GdkRiscosDisplay *rdisplay,
			       wimp_block *poll_block)
{
  wimp_entering *riscos_event = &poll_block->entering;
  wimp_pointer mouse;
  GdkWindow *gdk_window;
  GdkEvent *gdk_event;

  gdk_window = g_hash_table_lookup (rdisplay->id_ht,
				    riscos_event->w);
  if (gdk_window == NULL)
    return;

  xwimp_get_pointer_info (&mouse);

  guint modifiers = gdk_riscos_read_modifiers ();
  guint buttons = gdk_riscos_translate_buttons (mouse.buttons);

  _gdk_riscos_display_set_last_seen_time (rdisplay);

  gdk_event = gdk_event_new (GDK_ENTER_NOTIFY);
  gdk_event->crossing.window = g_object_ref (gdk_window);
  gdk_event->crossing.send_event = FALSE;
  gdk_event->crossing.subwindow = g_object_ref (gdk_window);
  gdk_event->crossing.time = rdisplay->last_seen_time;
  gdk_event->crossing.x = 0;
  gdk_event->crossing.y = 0;
  gdk_event->crossing.x_root = 0;
  gdk_event->crossing.y_root = 0;
  gdk_event->crossing.mode = GDK_CROSSING_NORMAL;
  gdk_event->crossing.detail = GDK_NOTIFY_UNKNOWN;
  gdk_event->crossing.focus = FALSE;
  gdk_event->crossing.state = modifiers | buttons;
  
  gdk_event_set_device (gdk_event, rdisplay->parent_instance.core_pointer);

  send_event (gdk_event);
}

static void
gdk_riscos_handle_leave_event (GdkRiscosDisplay *rdisplay,
			       wimp_block *poll_block)
{
  wimp_leaving *riscos_event = &poll_block->leaving;
  wimp_pointer mouse;
  GdkWindow *gdk_window;
  GdkEvent *gdk_event;

  gdk_window = g_hash_table_lookup (rdisplay->id_ht,
				    riscos_event->w);
  if (gdk_window == NULL)
    return;

  xwimp_get_pointer_info (&mouse);

  guint modifiers = gdk_riscos_read_modifiers ();
  guint buttons = gdk_riscos_translate_buttons (mouse.buttons);

  _gdk_riscos_display_set_last_seen_time (rdisplay);

  gdk_event = gdk_event_new (GDK_LEAVE_NOTIFY);
  gdk_event->crossing.window = g_object_ref (gdk_window);
  gdk_event->crossing.send_event = FALSE;
  gdk_event->crossing.subwindow = g_object_ref (gdk_window);
  gdk_event->crossing.time = rdisplay->last_seen_time;
  gdk_event->crossing.x = 0;
  gdk_event->crossing.y = 0;
  gdk_event->crossing.x_root = 0;
  gdk_event->crossing.y_root = 0;
  gdk_event->crossing.mode = GDK_CROSSING_NORMAL;
  gdk_event->crossing.detail = GDK_NOTIFY_UNKNOWN;
  gdk_event->crossing.focus = FALSE;
  gdk_event->crossing.state = modifiers | buttons;

  gdk_event_set_device (gdk_event, rdisplay->parent_instance.core_pointer);

  send_event (gdk_event);
}

/*
 * Need a better container for key codes, perhaps a hashmap?
 * Return -1 if there is no mapping.  */
static int keyboard_map_wimp_to_internal (int wimp_key)
{
  guint result = -1;

  int i;
  for (i = 0; i < sizeof (internal_keymap) / sizeof (struct internal_mapping); i++)
    if (internal_keymap[i].wimp_code == wimp_key)
      {
	result = internal_keymap[i].internal_code;
	break;
      }

  return result;
}

static guint
map_keyboard_wimp_to_gdk (int key, guint modifiers)
{
  guint result = (guint)key;

  if (key <= 26 && modifiers & GDK_CONTROL_MASK)
    return key + 'a' - 1;

  int i;
  for (i = 0; i < sizeof (function_keymap) / sizeof (struct keymapping); i++)
    if (function_keymap[i].riscos_key_code == key)
      {
	result = function_keymap[i].gdk_key_code;
	break;
      }

  return result;
}

#if 0
static void dump_keys_down(GdkRiscosDisplay *rdisplay)
{
  GSList *iterator = rdisplay->key_down_list;

  log_printf("  key down list:");
  while (iterator)
    {
      GSList *next = iterator->next;
      int code = GPOINTER_TO_INT (iterator->data);
      int inkey_code = keyboard_map_wimp_to_internal (code);

      printf("    inkey (%d), code (%d, '%c')",inkey_code,code,code);
      iterator = next;
    }
}
#endif

/* Clean out from the list any keys that were released while not over a
   GDK window.
   FIXME: Should we be checking for a grab window and sending an event to it?  */
static void
check_key_down_no_window (GdkRiscosDisplay *rdisplay)
{
  GSList *iterator = rdisplay->key_down_list;

  while (iterator)
    {
      GSList *next = iterator->next;
      int code = GPOINTER_TO_INT (iterator->data);
      int inkey_code = keyboard_map_wimp_to_internal (code);

      if (inkey_code >= 0)
	{
	  if (osbyte1 (osbyte_IN_KEY, inkey_code ^ 0xff, 0xff) == 0)
	    rdisplay->key_down_list = g_slist_delete_link (rdisplay->key_down_list, iterator);
	}
      else if (inkey_code == -1)
	{
	  /* FIXME: There shouldn't be any unidentifed keys, for now just remove them from
	   * the list.  */
	  rdisplay->key_down_list = g_slist_delete_link (rdisplay->key_down_list, iterator);
	}
      iterator = next;
    }
}

static void
check_key_down (GdkRiscosDisplay *rdisplay,
		GdkWindow *gdk_window,
		guint state)
{
  GSList *iterator = rdisplay->key_down_list;

  while (iterator)
    {
      GSList *next = iterator->next;
      int code = GPOINTER_TO_INT (iterator->data);
      int inkey_code = keyboard_map_wimp_to_internal (code);

      if (inkey_code >= 0 && osbyte1 (osbyte_IN_KEY, inkey_code ^ 0xff, 0xff) == 0)
	  {
	    _gdk_riscos_display_set_last_seen_time (rdisplay);

	    GdkEvent *event = gdk_event_new (GDK_KEY_RELEASE);
	    event->key.window = g_object_ref (gdk_window);
	    event->key.send_event = FALSE;
	    event->key.time = rdisplay->last_seen_time;
	    event->key.state = state;
	    event->key.keyval = map_keyboard_wimp_to_gdk (code, state);
	    event->key.length = 0;
	    event->key.string = NULL;
	    event->key.hardware_keycode = event->key.keyval;
	    event->key.group = 0;
	    event->key.is_modifier = FALSE;

	    gdk_event_set_device (event, rdisplay->parent_instance.core_pointer);

	    send_event (event);
#if 0
	    {
	      char buffer[150];
	      
	      sprintf (buffer, "KEY_RELEASE inkey (%d), wimp (%d) state (%X) '%c'",inkey_code, code,event->key.state, code);
	      report_text0 (buffer);
	    }
#endif
	    rdisplay->key_down_list = g_slist_delete_link (rdisplay->key_down_list, iterator);
	  }
	else if (inkey_code == -1)
	  {
	    /* FIXME: There shouldn't be any unidentifed keys, for now just remove them from
	     * the list.  */
	    rdisplay->key_down_list = g_slist_delete_link (rdisplay->key_down_list, iterator);
	  }

      iterator = next;
    }
}

static void
gdk_riscos_handle_null_event (GdkRiscosDisplay *rdisplay)
{
  GdkEvent *event;
  wimp_pointer mouse;

  /* If the task has no windows, then nothing to do here.  */
  if (rdisplay->toplevels == NULL)
    return;

  xwimp_get_pointer_info (&mouse);
  
  GdkWindow *gdk_window = g_hash_table_lookup (rdisplay->id_ht, mouse.w);
  if (!gdk_window)
    {
      check_key_down_no_window (rdisplay);
      return;
    }

  GdkWindowImplRiscos *impl = GDK_WINDOW_IMPL_RISCOS (gdk_window->impl);

  GdkPoint mouse_pos, global_pos, local_pos;
  gdk_riscos_screen_point_to_pixel (impl->screen, (GdkPoint *)&mouse.pos, &mouse_pos);
  gdk_riscos_screen_global_point (impl->screen, &mouse_pos, &global_pos);
  gdk_riscos_window_map_from_global (impl->wrapper, &mouse_pos, &local_pos);

  guint modifiers = gdk_riscos_read_modifiers ();
  guint buttons = gdk_riscos_translate_buttons (mouse.buttons);
  guint old_buttons = gdk_riscos_translate_buttons (rdisplay->last_click.buttons);

  check_key_down (rdisplay,
		  gdk_window,
		  modifiers | buttons);

  if (rdisplay->system_drag)
    {
      if (old_buttons != 0 && buttons == 0)
	rdisplay->system_drag = FALSE;
      return;
    }
  else
    {
      if (old_buttons != 0 && buttons == 0)
	{
	  _gdk_riscos_display_set_last_seen_time (rdisplay);

	  event = create_button_event (rdisplay,
				       gdk_window,
				       GDK_BUTTON_RELEASE);

	  event->button.x = local_pos.x;
	  event->button.y = local_pos.y;
	  event->button.state = modifiers;
	  event->button.button = translate_mouse_button (rdisplay->last_click.buttons);
	  event->button.x_root = global_pos.x;
	  event->button.y_root = global_pos.y;

	  send_event (event);

	  rdisplay->last_click.buttons = 0;

	  return;
	}

      /* Check for the user starting a window resize drag.  */
      guint current_button = translate_mouse_button (mouse.buttons);
      
      if (current_button == GDK_BUTTON_PRIMARY && modifiers & GDK_SHIFT_MASK)
	{
	  start_resize_window (rdisplay, &mouse);
	  return;
	}
    }

  /* If the pointer hasn't moved, then don't send a motion event.  */
  if (mouse_pos.x == rdisplay->last_mouse_move.x &&
      mouse_pos.y == rdisplay->last_mouse_move.y)
    return;

  rdisplay->last_mouse_move.x = mouse_pos.x;
  rdisplay->last_mouse_move.y = mouse_pos.y;

  /* Check if this window has requested to be notified of motion events.
     TODO: Reduce the number of events for MOTION_HINT and check for button
     presses for BUTTON_MOTION masks.  */
  GdkEventMask event_mask = gdk_window_get_events (gdk_window) | impl->event_mask;
  if ((event_mask & (GDK_POINTER_MOTION_MASK |
		    GDK_POINTER_MOTION_HINT_MASK |
		    GDK_BUTTON_MOTION_MASK |
		    GDK_BUTTON1_MOTION_MASK |
		    GDK_BUTTON2_MOTION_MASK |
		    GDK_BUTTON3_MOTION_MASK)) == 0)
    return;

  /* If the local_pos.y is -ve, then don't send a motion event; this can happen when
     dragging the window around by the title bar.  */
  if (local_pos.y < 0)
    return;

  _gdk_riscos_display_set_last_seen_time (rdisplay);

  event = gdk_event_new (GDK_MOTION_NOTIFY);
  event->motion.window = g_object_ref (gdk_window);
  event->motion.send_event = FALSE;
  event->motion.time = rdisplay->last_seen_time;
  event->motion.x = local_pos.x;
  event->motion.y = local_pos.y;
  event->motion.axes = NULL;
  event->motion.state = modifiers | buttons;
  event->motion.is_hint = FALSE;
  event->motion.device = rdisplay->parent_instance.core_pointer;
  event->motion.x_root = global_pos.x;
  event->motion.y_root = global_pos.y;
#if 0
{
  char buffer[150];
	      
  sprintf (buffer, "MOTION_NOTIFY : local(%f,%f) : global(%f,%f)",event->motion.x,event->motion.y,event->motion.x_root,event->motion.y_root);
  report_text0 (buffer);
}
#endif
  send_event (event);
}

static void
gdk_riscos_handle_key_press (GdkRiscosDisplay *rdisplay,
			     wimp_block *poll_block)
{
  wimp_key *key = &poll_block->key;

  GdkWindow *gdk_window = g_hash_table_lookup (rdisplay->id_ht, key->w);
  if (!gdk_window)
    return;

  wimp_pointer mouse;
  xwimp_get_pointer_info (&mouse);

  guint modifiers = gdk_riscos_read_modifiers ();
  guint buttons = gdk_riscos_translate_buttons (mouse.buttons);

  _gdk_riscos_display_set_last_seen_time (rdisplay);

  GdkEvent *event = gdk_event_new (GDK_KEY_PRESS);
  event->key.window = g_object_ref (gdk_window);
  event->key.send_event = FALSE;
  event->key.time = rdisplay->last_seen_time;
  event->key.state = modifiers | buttons;
  event->key.keyval = map_keyboard_wimp_to_gdk (key->c, modifiers);
  event->key.length = 0;
  event->key.string = NULL;
  event->key.hardware_keycode = event->key.keyval;
  event->key.group = 0;
  event->key.is_modifier = FALSE;

  gdk_event_set_device (event, rdisplay->parent_instance.core_pointer);

  send_event (event);

#if 0
    {
      char buffer[150];
      
      sprintf (buffer, "KEY_PRESS wimp (%d) state (%X) '%c'",key->c,event->key.state, key->c);
      report_text0 (buffer);
    }
#endif
  rdisplay->key_down_list = g_slist_prepend (rdisplay->key_down_list,
					     GUINT_TO_POINTER (key->c));
}

static gboolean
gdk_event_prepare (GSource *source,
		   gint    *timeout)
{
//  gdk_threads_enter ();
  
  *timeout = 0;

//  gdk_threads_leave ();

  return TRUE;
}

static gboolean
gdk_event_check (GSource *source)
{
  return TRUE;
}

static gboolean
gdk_event_dispatch (GSource     *source,
		    GSourceFunc  callback,
		    gpointer     user_data)
{
  GdkDisplay *display = ((GdkEventSource*) source)->display;
  GdkEvent *event;
  
  gdk_threads_enter ();

  event = gdk_display_get_event (display);

  if (event)
    {
      _gdk_event_emit (event);

      gdk_event_free (event);
    }

  gdk_threads_leave ();

  return TRUE;
}

static GSourceFuncs event_funcs = {
  gdk_event_prepare,
  gdk_event_check,
  gdk_event_dispatch,
  NULL
};

static void
_gdk_riscos_event_loop_init (GdkDisplay *display)
{
  GSource *source;
  GdkEventSource *event_source;

  /* Hook into the GLib main loop */

//  event_poll_fd.events = G_IO_IN;
//  event_poll_fd.fd = -1;

  source = g_source_new (&event_funcs, sizeof (GdkEventSource));
  g_source_set_name (source, "GDK RISC OS WIMP Event source"); 
  event_source = (GdkEventSource *) source;
  event_source->display = display;

//  g_source_add_poll (source, &event_poll_fd);

  /* Using a priority of GDK_PRIORITY_EVENTS prevents the widget expose
   * signals from getting through, so use a lower priority.  */
  g_source_set_priority (source, GDK_PRIORITY_REDRAW + 5);
  g_source_set_can_recurse (source, TRUE);
  g_source_attach (source, NULL);
}

static wimp_event_no
poll_wimp_event (GdkRiscosDisplay *rdisplay,
		 wimp_block *poll_block)
{
  wimp_event_no event_no;

  xwimp_poll (0, poll_block, NULL, &event_no);

  if (!call_raw_event_handlers (rdisplay, poll_block, event_no))
    {
      switch (event_no)
      {
      case wimp_NULL_REASON_CODE:
	gdk_riscos_handle_null_event (rdisplay);
	break;
      case wimp_REDRAW_WINDOW_REQUEST:
	gdk_riscos_handle_redraw_event (rdisplay, poll_block);
	break;
      case wimp_OPEN_WINDOW_REQUEST:
	gdk_riscos_handle_open_event (rdisplay, poll_block);
	break;
      case wimp_CLOSE_WINDOW_REQUEST:
	gdk_riscos_handle_close_event (rdisplay, poll_block);
	break;
      case wimp_POINTER_ENTERING_WINDOW:
	gdk_riscos_handle_enter_event (rdisplay, poll_block);
	break;
      case wimp_POINTER_LEAVING_WINDOW:
	gdk_riscos_handle_leave_event (rdisplay, poll_block);
	break;
      case wimp_MOUSE_CLICK:
	gdk_riscos_handle_mouse_click (rdisplay, poll_block);
	break;
      case wimp_KEY_PRESSED:
	gdk_riscos_handle_key_press (rdisplay, poll_block);
	break;
#if 0
      /* NOTE: The RISC OS clipboard model states that we should ignore
       * the wimp_LOSE_CARET/wimp_GAIN_CARET events and use message_CLAIM_ENTITY
       * instead.  */
      case wimp_LOSE_CARET:
	gdk_riscos_handle_lose_focus_event (rdisplay, poll_block);
	break;
      case wimp_GAIN_CARET:
	gdk_riscos_handle_gain_focus_event (rdisplay, poll_block);
	break;
#endif
      case wimp_USER_MESSAGE:
      case wimp_USER_MESSAGE_RECORDED:
      case wimp_USER_MESSAGE_ACKNOWLEDGE:
	gdk_riscos_handle_message (event_no, rdisplay, poll_block);
	break;
      }
    }

  return event_no;
}

void
_gdk_riscos_display_queue_events (GdkDisplay *display)
{
  GdkRiscosDisplay *riscos_display = GDK_RISCOS_DISPLAY (display);

  poll_wimp_event (riscos_display, &riscos_display->poll_block);

  pthread_yield();
}

wimp_event_no
gdk_riscos_event_poll (wimp_block *poll_block)
{
  GdkRiscosDisplay *riscos_display = GDK_RISCOS_DISPLAY(_gdk_display);

  return poll_wimp_event (riscos_display, poll_block);
}

void
_gdk_riscos_events_init (GdkDisplay *display)
{
  _gdk_riscos_event_loop_init (display);
}
