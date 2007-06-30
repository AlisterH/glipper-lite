import os, time
from os.path import *
import gnomeapplet, gtk, gtk.gdk, gconf, gnomevfs, gobject, gnome
from gettext import gettext as _

import glipper, glipper.About, glipper.Properties
from glipper.Keybinder import *
from glipper.History import *
from glipper.Clipboards import *
from glipper.PluginsManager import *

class Applet(object):
   def __init__(self, applet):
      self.applet = applet
      self.menu = gtk.Menu()
      self.tooltips = gtk.Tooltips()
      self.image = gtk.Image()
      self.image.set_from_pixbuf(gtk.IconTheme.load_icon(gtk.icon_theme_get_default(), "glipper", self.applet.get_size() - 2, 0))
      self.tooltips.set_tip(self.applet, _("Glipper - Popup shortcut: ") + get_glipper_keybinder().get_key_combination())
      
      self.mark_default_entry = glipper.GCONF_CLIENT.get_bool(glipper.GCONF_MARK_DEFAULT_ENTRY)
      if self.mark_default_entry == None:
         self.mark_default_entry = True
      glipper.GCONF_CLIENT.notify_add(glipper.GCONF_MARK_DEFAULT_ENTRY, lambda x, y, z, a: self.on_mark_default_entry_changed (z.value))
      
      self.save_history = glipper.GCONF_CLIENT.get_bool(glipper.GCONF_SAVE_HISTORY)
      if self.mark_default_entry == None:
         self.mark_default_entry = True
      glipper.GCONF_CLIENT.notify_add(glipper.GCONF_SAVE_HISTORY, lambda x, y, z, a: self.on_save_history_changed (z.value))
      
      self.max_item_length = glipper.GCONF_CLIENT.get_int(glipper.GCONF_MAX_ITEM_LENGTH)
      if self.max_item_length == None:
         self.max_elements = 35
      glipper.GCONF_CLIENT.notify_add(glipper.GCONF_MAX_ITEM_LENGTH, lambda x, y, z, a: self.on_max_item_length_changed (z.value))
      
      gtk.window_set_default_icon_name("glipper")
      
      get_glipper_keybinder().connect('activated', self.on_key_combination_press)
      get_glipper_keybinder().connect('changed', self.on_key_combination_changed)
      get_glipper_history().connect('changed', self.on_history_changed)
      get_glipper_history().load()
      get_glipper_plugins_manager().load()
      get_glipper_plugins_manager().connect('menu-items-changed', self.on_plugins_menu_items_changed)
      
      self.applet.connect('change-size', lambda applet, orient: self.on_change_size(applet))
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
      self.applet.set_flags(gnomeapplet.EXPAND_MINOR)
      self.applet.show_all()
   
   def on_plugins_menu_items_changed(self, manager):
      self.update_menu(get_glipper_history().get_history())
   
   def on_menu_deactivate(self, menu):
      self.applet.set_state(gtk.STATE_NORMAL)
   
   def on_menu_item_activate(self, menuitem, item):
      get_glipper_history().set_default(item)
      
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
      self.menu.destroy()
      self.menu = gtk.Menu()
      
      if len(history) == 0:
         self.menu.append(gtk.MenuItem(_('< Empty history >')))
      else:
         for item in history:
            menu_item = gtk.MenuItem(item, False)
            
            if len(item) > self.max_item_length:
	       i = item.replace("\n", " ")
	       i = item.replace("\t", " ")
	       short = i[0:self.max_item_length/2] + '...' + i[-(self.max_item_length/2-3):]
               menu_item.get_child().set_text(short)
               self.tooltips.set_tip(menu_item, item)

            if self.mark_default_entry and item == get_glipper_clipboards().get_default_clipboard_text():
               menu_item.get_child().set_markup('<b>' + menu_item.get_child().get_text() + '</b>')
               
            menu_item.connect('activate', self.on_menu_item_activate, item)
            self.menu.append(menu_item)
      
      self.menu.append(gtk.SeparatorMenuItem())
      
      clear_item = gtk.ImageMenuItem(gtk.STOCK_CLEAR)
      clear_item.connect('activate', self.on_clear)
      self.menu.append(clear_item)
      
      plugins_menu_items = get_glipper_plugins_manager().get_menu_items()
      
      if len(plugins_menu_items) > 0:
         self.menu.append(gtk.SeparatorMenuItem())
         
         for menu_item in plugins_menu_items:
            self.menu.append(plugins_menu_items[menu_item])
            
      self.menu.connect('deactivate', self.on_menu_deactivate)
      
      self.menu.show_all()

   def on_properties (self, component, verb):
      glipper.Properties.Properties(self.applet)
      
   def on_help (self, component, verb):
      gnome.help_display('glipper')
   
   def on_about (self, component, verb):
	   glipper.About.show_about(self.applet)
   
   def on_plugins (self, component, verb):
      PluginsWindow(self.applet)
   
   def on_clicked(self, applet, event):
      if event.button != 1:
         return False
         
      self.popup_menu()
      
      return True
      
   def on_destroy(self, applet):
      if self.save_history:
         get_glipper_history().save()
      get_glipper_plugins_manager().call('stop')
      
   def on_change_size(self, applet):
      icon_size = applet.get_size() - 2 # Padding

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
         
   def on_history_changed(self, object, history):
      self.update_menu(history)
      get_glipper_plugins_manager().call('historyChanged')
      
   def on_mark_default_entry_changed(self, value):
      if value is None or value.type != gconf.VALUE_BOOL:
         return
      self.mark_default_entry = value.get_bool()
      self.update_menu(get_history().get_history())
      
   def on_save_history_changed(self, value):
      if value is None or value.type != gconf.VALUE_BOOL:
         return
      self.save_history = value.get_bool()
      
   def on_max_item_length_changed (self, value):
      if value is None or value.type != gconf.VALUE_INT:
         return
      self.max_item_length = value.get_int()
      self.update_menu(get_history().get_history())
