/* Glipper - Clipboardmanager for GNOME
 * Copyright (C) 2007 Glipper Team
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

import gobject, gtk, gconf
import glipper
from glipper.Clipboards import *
from glipper.PluginsManager import *
from gettext import gettext as _

class History(gobject.GObject):
   __gsignals__ = {
      "changed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, [gobject.TYPE_PYOBJECT]),
   }
	
   def __init__(self):
      gobject.GObject.__init__(self)
      self.history = []
      get_glipper_clipboards().connect('new-item', self.on_new_item)

      self.max_elements = glipper.GCONF_CLIENT.get_int(glipper.GCONF_MAX_ELEMENTS)
      if self.max_elements == None:
         self.max_elements = 20
      glipper.GCONF_CLIENT.notify_add(glipper.GCONF_MAX_ELEMENTS, lambda x, y, z, a: self.on_max_elements_changed (z.value))
   
   def get_history(self):
      return self.history
   
   def on_new_item(self, clipboards, item):
      self.add(item)
      get_glipper_plugins_manager().call('on_new_item', item)
      
   def clear(self):
      self.history = []
      self.emit('changed', self.history)
   
   def get(self, index):
      if index >= len(self.history):
         return

      return self.history[index]
   
   def set(self, index, item):
      if item in self.history:
         self.history.remove(item)

      if index == len(self.history):
         self.history.append(item)
      else:
         self.history[index] = item 
      self.emit('changed', self.history)
   
   def add(self, item):
      if item in self.history:
         self.history.remove(item)
         
      self.history.insert(0, item)
      if len(self.history) > self.max_elements:
         self.history = self.history[0:self.max_elements]
      self.emit('changed', self.history)

   def remove(self, index):
      del self.history[index]

   def load(self):
      try:
         file = open(glipper.HISTORY_FILE, "r")
      except IOError:
         self.emit('changed', self.history)
         return # Cannot read history file
      
      length = file.readline()
      while length:
         self.history.append(file.read(int(length)))
         file.read(1) # This is for \n
         length = file.readline()
      
      file.close()
      self.emit('changed', self.history)
      
   def save(self):
      try:
         file = open(glipper.HISTORY_FILE, "w")
      except IOError:
         return # Cannot write to history file
         
      for item in self.history:
         file.write(str(len(item)) + '\n')
         file.write(item + '\n')
      
      file.close()
   
   def on_max_elements_changed (self, value):
      if value is None or value.type != gconf.VALUE_INT:
         return
      self.max_elements = value.get_int()
      if len(self.history) > self.max_elements:
         self.history = self.history[0:self.max_elements]
         self.emit('changed', self.history)
         
history = History()

def get_glipper_history():
   return history
