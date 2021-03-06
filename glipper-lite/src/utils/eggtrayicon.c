/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* eggtrayicon.c
 * Copyright (C) 2002 Anders Carlsson <andersca@gnu.org>
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

// File taken from gdesklets with patch from :
// http://bugzilla.gnome.org/attachment.cgi?id=65146&action=view

#include "eggtrayicon.h"

#include <X11/Xatom.h>

#define SYSTEM_TRAY_REQUEST_DOCK    0

#define SYSTEM_TRAY_ORIENTATION_HORZ 0
#define SYSTEM_TRAY_ORIENTATION_VERT 1

enum {
  PROP_0,
  PROP_ORIENTATION
};

G_DEFINE_TYPE (EggTrayIcon, egg_tray_icon, GTK_TYPE_PLUG);

static void egg_tray_icon_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec);

static void egg_tray_icon_realize   (GtkWidget *widget);
static void egg_tray_icon_unrealize (GtkWidget *widget);

static void egg_tray_icon_add (GtkContainer *container,
			       GtkWidget    *widget);


static void egg_tray_icon_update_manager_window (EggTrayIcon *icon);

static void
egg_tray_icon_init (EggTrayIcon *icon)
{
  icon->stamp = 1;
  icon->orientation = GTK_ORIENTATION_HORIZONTAL;

  gtk_widget_add_events (GTK_WIDGET (icon), GDK_PROPERTY_CHANGE_MASK);
}

static void
egg_tray_icon_class_init (EggTrayIconClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;
  GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;
  GtkContainerClass *container_class = (GtkContainerClass *)klass;

  gobject_class->get_property = egg_tray_icon_get_property;

  widget_class->realize   = egg_tray_icon_realize;
  widget_class->unrealize = egg_tray_icon_unrealize;
  
  container_class->add = egg_tray_icon_add;

  g_object_class_install_property (gobject_class,
                                   PROP_ORIENTATION,
                                   g_param_spec_enum ("orientation",
                                                      "Orientation",
                                                      "The orientation of the tray.",
                                                      GTK_TYPE_ORIENTATION,
                                                      GTK_ORIENTATION_HORIZONTAL,
                                                      G_PARAM_READABLE));

}

static void
egg_tray_icon_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  EggTrayIcon *icon = EGG_TRAY_ICON (object);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, icon->orientation);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
egg_tray_icon_get_orientation_property (EggTrayIcon *icon)
{
  Display *xdisplay;
  Atom type;
  int format;
  union {
        gulong *prop;
        guchar *prop_ch;
  } prop = { NULL };
  gulong nitems;
  gulong bytes_after;
  int error, result;

  g_assert (icon->manager_window != None);

  xdisplay = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));

  gdk_error_trap_push ();
  type = None;
  result = XGetWindowProperty (xdisplay,
                               icon->manager_window,
                               icon->orientation_atom,
                               0, G_MAXLONG, FALSE,
                               XA_CARDINAL,
                               &type, &format, &nitems,
                               &bytes_after, &(prop.prop_ch));
  error = gdk_error_trap_pop ();

  if (error || result != Success)
    return;

  if (type == XA_CARDINAL)
    {
      GtkOrientation orientation;

      orientation = (prop.prop [0] == SYSTEM_TRAY_ORIENTATION_HORZ) ?
                                        GTK_ORIENTATION_HORIZONTAL :
                                        GTK_ORIENTATION_VERTICAL;

      if (icon->orientation != orientation)
        {
          icon->orientation = orientation;

          g_object_notify (G_OBJECT (icon), "orientation");
        }
    }

  if (prop.prop)
    XFree (prop.prop);
}

static GdkFilterReturn
egg_tray_icon_manager_filter (GdkXEvent *xevent, GdkEvent *event, gpointer user_data)
{
  EggTrayIcon *icon = user_data;
  XEvent *xev = (XEvent *)xevent;

  if (xev->xany.type == ClientMessage &&
      xev->xclient.message_type == icon->manager_atom &&
      xev->xclient.data.l[1] == icon->selection_atom)
    {
      egg_tray_icon_update_manager_window (icon);
    }
  else if (xev->xany.window == icon->manager_window)
    {
      if (xev->xany.type == PropertyNotify &&
          xev->xproperty.atom == icon->orientation_atom)
        {
          egg_tray_icon_get_orientation_property (icon);
        }
      if (xev->xany.type == DestroyNotify)
        {
          egg_tray_icon_update_manager_window (icon);
        }
    }

  return GDK_FILTER_CONTINUE;
}

static void
egg_tray_icon_unrealize (GtkWidget *widget)
{
  EggTrayIcon *icon = EGG_TRAY_ICON (widget);
  GdkWindow *root_window;

  if (icon->manager_window != None)
    {
      GdkWindow *gdkwin;

      gdkwin = gdk_window_lookup_for_display (gtk_widget_get_display (widget),
                                              icon->manager_window);

      gdk_window_remove_filter (gdkwin, egg_tray_icon_manager_filter, icon);
    }

  root_window = gdk_screen_get_root_window (gtk_widget_get_screen (widget));

  gdk_window_remove_filter (root_window, egg_tray_icon_manager_filter, icon);

  if (GTK_WIDGET_CLASS (egg_tray_icon_parent_class)->unrealize)
    (* GTK_WIDGET_CLASS (egg_tray_icon_parent_class)->unrealize) (widget);
}

static void
egg_tray_icon_send_manager_message (EggTrayIcon *icon,
                                    long         message,
                                    Window       window,
                                    long         data1,
                                    long         data2,
                                    long         data3)
{
  XClientMessageEvent ev;
  Display *display;

  ev.type = ClientMessage;
  ev.window = window;
  ev.message_type = icon->system_tray_opcode_atom;
  ev.format = 32;
  ev.data.l[0] = gdk_x11_get_server_time (GTK_WIDGET (icon)->window);
  ev.data.l[1] = message;
  ev.data.l[2] = data1;
  ev.data.l[3] = data2;
  ev.data.l[4] = data3;

  display = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));

  gdk_error_trap_push ();
  XSendEvent (display,
              icon->manager_window, False, NoEventMask, (XEvent *)&ev);
  XSync (display, False);
  gdk_error_trap_pop ();
}

static void
egg_tray_icon_send_dock_request (EggTrayIcon *icon)
{
  egg_tray_icon_send_manager_message (icon,
                                      SYSTEM_TRAY_REQUEST_DOCK,
                                      icon->manager_window,
                                      gtk_plug_get_id (GTK_PLUG (icon)),
                                      0, 0);
}

static gboolean
transparent_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
  gdk_window_clear_area (widget->window, event->area.x, event->area.y,
			 event->area.width, event->area.height);
  return FALSE;
}

static void
make_transparent_again (GtkWidget *widget, GtkStyle *previous_style,
			gpointer user_data)
{
  gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
}

static void
make_transparent (GtkWidget *widget, gpointer user_data)
{
  if (GTK_WIDGET_NO_WINDOW (widget) || GTK_WIDGET_APP_PAINTABLE (widget))
    return;

  gtk_widget_set_app_paintable (widget, TRUE);
  gtk_widget_set_double_buffered (widget, FALSE);
  gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
  g_signal_connect (widget, "expose_event",
		    G_CALLBACK (transparent_expose_event), NULL);
  g_signal_connect_after (widget, "style_set",
			  G_CALLBACK (make_transparent_again), NULL);
}	


static void
egg_tray_icon_update_manager_window (EggTrayIcon *icon)
{
  Display *xdisplay;

  xdisplay = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));

  if (icon->manager_window != None)
    {
      GdkWindow *gdkwin;

      gdkwin = gdk_window_lookup_for_display (gtk_widget_get_display (GTK_WIDGET (icon)),
                                              icon->manager_window);

      gdk_window_remove_filter (gdkwin, egg_tray_icon_manager_filter, icon);
    }

  XGrabServer (xdisplay);

  icon->manager_window = XGetSelectionOwner (xdisplay,
                                             icon->selection_atom);

  if (icon->manager_window != None)
    XSelectInput (xdisplay,
                  icon->manager_window, StructureNotifyMask|PropertyChangeMask);

  XUngrabServer (xdisplay);
  XFlush (xdisplay);

  if (icon->manager_window != None)
    {
      GdkWindow *gdkwin;

      gdkwin = gdk_window_lookup_for_display (gtk_widget_get_display (GTK_WIDGET (icon)),
                                              icon->manager_window);

      gdk_window_add_filter (gdkwin, egg_tray_icon_manager_filter, icon);

      /* Send a request that we'd like to dock */
      egg_tray_icon_send_dock_request (icon);

      egg_tray_icon_get_orientation_property (icon);
    }
}

static void
egg_tray_icon_realize (GtkWidget *widget)
{
  EggTrayIcon *icon = EGG_TRAY_ICON (widget);
  GdkScreen *screen;
  GdkDisplay *display;
  Display *xdisplay;
  char buffer[256];
  GdkWindow *root_window;

  if (GTK_WIDGET_CLASS (egg_tray_icon_parent_class)->realize)
    GTK_WIDGET_CLASS (egg_tray_icon_parent_class)->realize (widget);
    
  make_transparent (widget, NULL);

  screen = gtk_widget_get_screen (widget);
  display = gdk_screen_get_display (screen);
  xdisplay = gdk_x11_display_get_xdisplay (display);

  /* Now see if there's a manager window around */
  g_snprintf (buffer, sizeof (buffer),
              "_NET_SYSTEM_TRAY_S%d",
              gdk_screen_get_number (screen));

  icon->selection_atom = XInternAtom (xdisplay, buffer, False);

  icon->manager_atom = XInternAtom (xdisplay, "MANAGER", False);

  icon->system_tray_opcode_atom = XInternAtom (xdisplay,
                                               "_NET_SYSTEM_TRAY_OPCODE",
                                               False);

  icon->orientation_atom = XInternAtom (xdisplay,
                                        "_NET_SYSTEM_TRAY_ORIENTATION",
                                        False);

  egg_tray_icon_update_manager_window (icon);

  root_window = gdk_screen_get_root_window (screen);

  /* Add a root window filter so that we get changes on MANAGER */
  gdk_window_add_filter (root_window,
                         egg_tray_icon_manager_filter, icon);
}

EggTrayIcon*
egg_tray_icon_new (const gchar *name)
{
  return g_object_new (EGG_TYPE_TRAY_ICON, "title", name, NULL);
}

GtkOrientation
egg_tray_icon_get_orientation (EggTrayIcon *icon)
{
  g_return_val_if_fail (EGG_IS_TRAY_ICON (icon), GTK_ORIENTATION_HORIZONTAL);

  return icon->orientation;
}

static void
egg_tray_icon_add (GtkContainer *container, GtkWidget *widget)
{
  g_signal_connect (widget, "realize",
		    G_CALLBACK (make_transparent), NULL);
  GTK_CONTAINER_CLASS (egg_tray_icon_parent_class)->add (container, widget);
}


