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

import gtk, gobject, gconf
import glipper, glipper.keybinder

class Keybinder(gobject.GObject):
   __gsignals__ = {
      "activated" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, [gobject.TYPE_ULONG]),
      # When the keybinding changes, passes a boolean indicating wether the keybinding is successful
      "changed" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, [gobject.TYPE_BOOLEAN]),
   }

   def __init__(self):
      gobject.GObject.__init__(self)
      
      self.bound = False
      self.prevbinding = None
      
      # Set and retreive global keybinding from gconf
      self.key_combination = glipper.GCONF_CLIENT.get_string(glipper.GCONF_KEY_COMBINATION)
      if self.key_combination == None:
         # This is for uninstalled cases, the real default is in the schema
         self.key_combination = "<Ctrl><Alt>C"
      glipper.GCONF_CLIENT.notify_add(glipper.GCONF_KEY_COMBINATION, lambda x, y, z, a: self.on_config_key_combination(z.value))
      
      self.bind()
      
   def on_config_key_combination(self, value=None):
      if value != None and value.type == gconf.VALUE_STRING:
         self.prevbinding = self.key_combination
         self.key_combination = value.get_string()
         self.bind()
   
   def on_keyboard_shortcut(self):
      self.emit('activated', glipper.keybinder.tomboy_keybinder_get_current_event_time())
   
   def get_key_combination(self):
      return self.key_combination
   
   def bind(self):
      if self.bound:
         self.unbind()
         
      try:
         print 'Binding shortcut %s to popup glipper' % self.key_combination
         glipper.keybinder.tomboy_keybinder_bind(self.key_combination, self.on_keyboard_shortcut)
         self.bound = True
      except KeyError:
         # if the requested keybinding conflicts with an existing one, a KeyError will be thrown
         self.bound = False
      
      self.emit('changed', self.bound)
               
   def unbind(self):
      try:
         glipper.keybinder.tomboy_keybinder_unbind(self.prevbinding)
         self.bound = False
      except KeyError:
         # if the requested keybinding is not bound, a KeyError will be thrown
         pass

if gtk.pygtk_version < (2,8,0):
   gobject.type_register(Keybinder)

keybinder = Keybinder()

def get_glipper_keybinder():
   return keybinder
