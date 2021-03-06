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

import gtk, gnomevfs, glipper, gobject, gconf, gnome
from gettext import gettext as _
from os.path import *
from glipper.Plugin import *

class PluginsManager(gobject.GObject):
   __gsignals__ = {
      "menu-items-changed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, []),
   }

   def __init__(self):
      gobject.GObject.__init__(self)
      self.plugins = []
      self.menu_items = [] # History menu items for plugins
      
      self.autostart_plugins = glipper.GCONF_CLIENT.get_list(glipper.GCONF_AUTOSTART_PLUGINS, gconf.VALUE_STRING)
      
      if self.autostart_plugins == None:
         self.autostart_plugins = ['nopaste']
      glipper.GCONF_CLIENT.notify_add(glipper.GCONF_AUTOSTART_PLUGINS, lambda x, y, z, a: self.on_autostart_plugins_changed (z.value))

   def on_autostart_plugins_changed(self, value):
      if value is None or value.type != gconf.VALUE_LIST:
         return
      self.autostart_plugins = glipper.GCONF_CLIENT.get_list(glipper.GCONF_AUTOSTART_PLUGINS, gconf.VALUE_STRING)

   def load(self):
      for file_name in self.autostart_plugins:
         self.start(file_name)

   def start(self, file_name):
      plugin = Plugin(file_name)
      self.plugins.append(plugin)
      self.init_plugin = plugin # Adding menu items from plugins is allowed only in the init function.
                                # add_menu_item uses this to know what plugin it is being called for.
      plugin.call('init')
      self.init_plugin = None

   def stop(self, file_name):
      for plugin in self.plugins:
         if plugin.get_file_name() == file_name:
            plugin.call('stop')
            plugin.remove_module()
            self.plugins.remove(plugin)
            self.remove_menu_items(plugin)
            
   def stop_all(self):
      for plugin in self.plugins:
         self.stop(plugin)
   
   def get_menu_items(self):
      return self.menu_items   
   
   def remove_menu_items(self, plugin):
      for p, menu_item in self.menu_items:
         if p == plugin:
            self.menu_items.remove((p, menu_item))

      self.emit('menu-items-changed')
   
   def add_menu_item(self, menu_item):
      self.menu_items.append((self.init_plugin, menu_item))
      self.emit('menu-items-changed')
      
   def get_started(self, file_name):
      for plugin in self.plugins:
         if plugin.get_file_name() == file_name:
            return True
      return False
      
   def get_autostarted(self, file_name):
      return file_name in self.autostart_plugins

   def call(self, signal, *args):
      for plugin in self.plugins:
         plugin.call(signal, *args)

class PluginsWindow(object):
   __instance = None

   def __init__(self, parent):
      if PluginsWindow.__instance == None:
         PluginsWindow.__instance = self
      else:
         PluginsWindow.__instance.plugins_window.present()
         return
         
      glade_file = gtk.glade.XML(join(glipper.SHARED_DATA_DIR, "plugins-window.glade"))
      
      self.FILE_NAME_COLUMN, self.ENABLED_COLUMN, self.AUTOSTART_COLUMN, self.NAME_COLUMN, self.DESCRIPTION_COLUMN, self.PREFERENCES_COLUMN = range(6)
      
      self.plugins_window = glade_file.get_widget("plugins_window")
      self.plugins_list = glade_file.get_widget("plugins_list")
      self.preferences_button = glade_file.get_widget("preferences_button")
      self.refresh_button = glade_file.get_widget("refresh_button")
      self.plugins_list_model = gtk.ListStore(gobject.TYPE_STRING, gobject.TYPE_BOOLEAN, gobject.TYPE_BOOLEAN, gobject.TYPE_STRING, gobject.TYPE_STRING, gobject.TYPE_BOOLEAN)
      self.plugins_list.set_model(self.plugins_list_model)
      self.plugins_list.get_selection().connect('changed', self.on_plugins_list_selection_changed)
      self.temp_plugins = []
      
      renderer = gtk.CellRendererToggle()
      renderer.connect('toggled', self.on_enabled_toggled)
      self.plugins_list.append_column(gtk.TreeViewColumn(_("Enabled"), renderer, active = self.ENABLED_COLUMN))
      renderer = gtk.CellRendererToggle()
      renderer.connect('toggled', self.on_autostart_toggled)
      self.plugins_list.append_column(gtk.TreeViewColumn(_("Autostart"), renderer, active = self.AUTOSTART_COLUMN))
      self.plugins_list.append_column(gtk.TreeViewColumn(_("Name"), gtk.CellRendererText(), text = self.NAME_COLUMN))
      self.plugins_list.append_column(gtk.TreeViewColumn(_("Description"), gtk.CellRendererText(), text = self.DESCRIPTION_COLUMN))
      
      self.autostart_plugins = glipper.GCONF_CLIENT.get_list(glipper.GCONF_AUTOSTART_PLUGINS, gconf.VALUE_STRING)
      if self.autostart_plugins == None:
         self.autostart_plugins = ['nopaste']
      self.autostart_plugins_notify = glipper.GCONF_CLIENT.notify_add(glipper.GCONF_AUTOSTART_PLUGINS, lambda x, y, z, a: self.on_autostart_plugins_changed (z.value))
      
      glade_file.signal_autoconnect(self)
      
      self.update_plugins_list_model()
      
      self.plugins_window.set_screen(parent.get_screen())
      self.plugins_window.show_all()

   def update_plugins_list_model(self):
      self.plugins_list_model.clear()
      directory = gnomevfs.DirectoryHandle(glipper.PLUGINS_DIR)
      
      for file in directory:
         if file.name[-3:] == ".py":
            try:
               plugin = Plugin(file.name[:-3])
               self.plugins_list_model.append([plugin.get_file_name(), plugins_manager.get_started(plugin.get_file_name()), plugins_manager.get_autostarted(plugin.get_file_name()), plugin.get_name(), plugin.get_description(), plugin.get_preferences()])
               if not plugins_manager.get_started(plugin.get_file_name()):
                  plugin.remove_module()
            except:
               print "Error trying to open plugin %s" % file.name
            
   def on_autostart_plugins_changed(self, value):
      if value is None or value.type != gconf.VALUE_LIST:
         return
      self.autostart_plugins = glipper.GCONF_CLIENT.get_list(glipper.GCONF_AUTOSTART_PLUGINS, gconf.VALUE_STRING)

      iter = self.plugins_list_model.get_iter_first()
      
      while iter:
         file_name = self.plugins_list_model.get_value(iter, self.FILE_NAME_COLUMN)
         
         if file_name in self.autostart_plugins:
            self.plugins_list_model.set_value(iter, self.AUTOSTART_COLUMN, True)
         else:
            self.plugins_list_model.set_value(iter, self.AUTOSTART_COLUMN, False)
            
         iter = self.plugins_list_model.iter_next(iter)
            
   def on_plugins_window_response(self, dialog, response):
      if response == gtk.RESPONSE_DELETE_EVENT or response == gtk.RESPONSE_CLOSE:
         dialog.destroy()
         glipper.GCONF_CLIENT.notify_remove(self.autostart_plugins_notify)
         
         for plugin in self.temp_plugins:
            if not plugins_manager.get_started(plugin.get_file_name()):
               plugin.remove_module()
         
         PluginsWindow.__instance = None
      elif response == 1:
         treeview, iter = self.plugins_list.get_selection().get_selected()
      
         file_name = self.plugins_list_model.get_value(iter, self.FILE_NAME_COLUMN)
      
         plugin = Plugin(file_name)
         self.temp_plugins.append(plugin)
         plugin.call('on_show_preferences', self.plugins_window)
      elif response == gtk.RESPONSE_HELP:
         gnome.help_display('glipper', 'plugins')
      
   def on_plugins_list_selection_changed(self, selection):
      treeview, iter = selection.get_selected()
      
      preferences = self.plugins_list_model.get_value(iter, self.PREFERENCES_COLUMN)
 
      self.preferences_button.set_sensitive(preferences)
      
   def on_autostart_toggled(self, renderer, path):
      iter = self.plugins_list_model.get_iter(path)
      file_name = self.plugins_list_model.get_value(iter, self.FILE_NAME_COLUMN)
      
      if plugins_manager.get_autostarted(file_name):
         self.autostart_plugins.remove(file_name)
      else:
         self.autostart_plugins.append(file_name)
      
      self.plugins_list_model.set_value(iter, self.AUTOSTART_COLUMN, plugins_manager.get_autostarted(file_name))
      glipper.GCONF_CLIENT.set_list(glipper.GCONF_AUTOSTART_PLUGINS, gconf.VALUE_STRING, self.autostart_plugins)
      
   def on_enabled_toggled(self, renderer, path):
      iter = self.plugins_list_model.get_iter(path)
      file_name = self.plugins_list_model.get_value(iter, self.FILE_NAME_COLUMN)
  
      if plugins_manager.get_started(file_name):
         plugins_manager.stop(file_name)
      else:
         plugins_manager.start(file_name)
      
      self.plugins_list_model.set_value(iter, self.ENABLED_COLUMN, plugins_manager.get_started(file_name))

plugins_manager = PluginsManager()

def get_glipper_plugins_manager():
   return plugins_manager
