# Glipper - Clipboardmanager for GNOME
# Copyright (C) 2007 Glipper Team
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#

import os, time
from os.path import *
import gnomeapplet, gtk, gtk.gdk, gconf, gnomevfs, gobject, gnome
from gettext import gettext as _

import glipper, glipper.About, glipper.Properties
from glipper.Keybinder import *
from glipper.History import *
from glipper.PluginsManager import *

class Applet(object):
   def __init__(self, applet):
      self.applet = applet
      self.size = 24
      self.menu = gtk.Menu()
      self.tooltips = gtk.Tooltips()
      self.image = gtk.Image()
      self.image.set_from_pixbuf(gtk.IconTheme.load_icon(gtk.icon_theme_get_default(), "glipper", self.size - 2, 0))
      self.tooltips.set_tip(self.applet, _("Glipper - Popup shortcut: ") + get_glipper_keybinder().get_key_combination())

      glipper.GCONF_CLIENT.notify_add(glipper.GCONF_MARK_DEFAULT_ENTRY, lambda x, y, z, a: self.update_menu(get_glipper_history().get_history()))
      glipper.GCONF_CLIENT.notify_add(glipper.GCONF_MAX_ITEM_LENGTH, lambda x, y, z, a: self.update_menu(get_glipper_history().get_history()))
      
      gtk.window_set_default_icon_name("glipper")
      
      get_glipper_keybinder().connect('activated', self.on_key_combination_press)
      get_glipper_keybinder().connect('changed', self.on_key_combination_changed)
      get_glipper_history().connect('changed', self.on_history_changed)
      get_glipper_plugins_manager().load()
      get_glipper_history().load()
      get_glipper_plugins_manager().connect('menu-items-changed', self.on_plugins_menu_items_changed)
      
      self.applet.connect('size-allocate', self.on_size_allocate)
      self.applet.connect('button-press-event', self.on_clicked)
      self.applet.connect('destroy', self.on_destroy)
      
      self.applet.setup_menu_from_file (
         glipper.SHARED_DATA_DIR, "Glipper.xml",
         None, [
         ("Properties", self.on_properties),
         ("Help", self.on_help),
         ("About", self.on_about),
         ("Plugins", self.on_plugins),
         ])

      self.applet.add(self.image)
      self.applet.set_applet_flags(gnomeapplet.EXPAND_MINOR)
      self.applet.show_all()
   
   def on_plugins_menu_items_changed(self, manager):
      self.update_menu(get_glipper_history().get_history())
   
   def on_menu_deactivate(self, menu):
      self.applet.set_state(gtk.STATE_NORMAL)
   
   def on_menu_item_activate(self, menuitem, item):
      get_glipper_clipboards().set_text(item)
      
   def on_clear(self, menuitem):
      get_glipper_history().clear()
   
   def position_menu(self, menu, data=None):
      x, y = self.applet.window.get_origin()
      
      requisition_width, requisition_height = self.menu.size_request()
      
      if y + self.applet.allocation.height + requisition_height > self.applet.get_screen().get_height():
         y -= requisition_height
         
      else:
         y += self.applet.allocation.height
      
      return x, y, True
   
   def popup_menu(self):
      self.applet.set_state(gtk.STATE_SELECTED)
      self.menu.popup(None, None, self.position_menu, 1, gtk.get_current_event_time())
   
   def update_menu(self, history):
      plugins_menu_items = get_glipper_plugins_manager().get_menu_items()
      
      for module, menu_item in plugins_menu_items:
         if menu_item.get_parent() == self.menu:
            self.menu.remove(menu_item)
         
      self.menu.destroy()
      self.menu = gtk.Menu()
         
      if len(history) == 0:
         menu_item = gtk.ImageMenuItem(gtk.STOCK_STOP)
         menu_item.get_child().set_text(_('Empty history'))
         self.menu.append(menu_item)
      else:
         for item in history:
            menu_item = gtk.MenuItem(format_item(item), False)

            if len(item) > max_item_length :
               self.tooltips.set_tip(menu_item, item[:glipper.MAX_TOOLTIPS_LENGTH])

            if mark_default_entry and item == get_glipper_clipboards().get_default_clipboard_text():
               menu_item.get_child().set_markup('<b>' + gobject.markup_escape_text(menu_item.get_child().get_text()) + '</b>')
               
            menu_item.connect('activate', self.on_menu_item_activate, item)
            self.menu.append(menu_item)
      
      self.menu.append(gtk.SeparatorMenuItem())
      
      clear_item = gtk.ImageMenuItem(gtk.STOCK_CLEAR)
      clear_item.connect('activate', self.on_clear)
      self.menu.append(clear_item)
      
      if len(plugins_menu_items) > 0:
         self.menu.append(gtk.SeparatorMenuItem())
         
         for module, menu_item in plugins_menu_items:
            self.menu.append(menu_item)
            
      self.menu.connect('deactivate', self.on_menu_deactivate)
      
      self.menu.show_all()

   def on_properties (self, component, verb):
      glipper.Properties.Properties(self.applet)
      
   def on_help (self, component, verb):
      gnome.help_display('glipper')
   
   def on_about (self, component, verb):
	   glipper.About.About(self.applet)
   
   def on_plugins (self, component, verb):
      PluginsWindow(self.applet)
   
   def on_clicked(self, applet, event):
      if event.button != 1:
         return False
         
      self.popup_menu()
      
      return True
      
   def on_destroy(self, applet):
      get_glipper_plugins_manager().stop_all()
      
   def on_size_allocate(self, applet, allocation):
      if allocation.height == self.size:
         return

      self.size = allocation.height

      icon_size = self.size - 2 # Padding
      # We do this to prevent icon scaling
      if icon_size <= 21:
         icon_size = 16
      elif icon_size <= 31:
         icon_size = 22
      elif icon_size <= 47:
         icon_size = 32
      
      self.image.set_from_pixbuf(gtk.IconTheme.load_icon(gtk.icon_theme_get_default(), "glipper", icon_size, 0))
      
   def on_key_combination_press(self, widget, time):
      self.menu.popup(None, None, None, 1, gtk.get_current_event_time())
   
   def on_key_combination_changed(self, keybinder, success):
      if success:
         self.tooltips.set_tip(self.applet, _("Glipper - Popup shortcut: ") + get_glipper_keybinder().get_key_combination())
         
   def on_history_changed(self, history, history_list):
      self.update_menu(history_list)
      get_glipper_plugins_manager().call('on_history_changed')
      if save_history:
         history.save()

# These variables and functions are available for all Applet instances:

mark_default_entry = glipper.GCONF_CLIENT.get_bool(glipper.GCONF_MARK_DEFAULT_ENTRY)
if mark_default_entry == None:
   mark_default_entry = True
glipper.GCONF_CLIENT.notify_add(glipper.GCONF_MARK_DEFAULT_ENTRY, lambda x, y, z, a: on_mark_default_entry_changed (z.value))

save_history = glipper.GCONF_CLIENT.get_bool(glipper.GCONF_SAVE_HISTORY)
if mark_default_entry == None:
   mark_default_entry = True
glipper.GCONF_CLIENT.notify_add(glipper.GCONF_SAVE_HISTORY, lambda x, y, z, a: on_save_history_changed (z.value))

max_item_length = glipper.GCONF_CLIENT.get_int(glipper.GCONF_MAX_ITEM_LENGTH)
if max_item_length == None:
   max_elements = 35
glipper.GCONF_CLIENT.notify_add(glipper.GCONF_MAX_ITEM_LENGTH, lambda x, y, z, a: on_max_item_length_changed (z.value))

def on_mark_default_entry_changed(value):
   global mark_default_entry
   if value is None or value.type != gconf.VALUE_BOOL:
      return
   mark_default_entry = value.get_bool()

def on_save_history_changed(value):
   global save_history
   if value is None or value.type != gconf.VALUE_BOOL:
      return
   save_history = value.get_bool()

def on_max_item_length_changed (value):
   global max_item_length
   if value is None or value.type != gconf.VALUE_INT:
      return
   max_item_length = value.get_int()

def format_item(item):
   i = item.replace("\n", " ")
   i = i.replace("\t", " ")
   if len(item) > max_item_length:
     return i[0:max_item_length/2] + '...' + i[-(max_item_length/2-3):]
   return i
