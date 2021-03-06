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

import gtk, gobject, glipper, gconf

class Clipboards(gobject.GObject):
   
   __gsignals__ = {
      "new-item" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, [gobject.TYPE_STRING]),
   }  
   
   def __init__(self):
      gobject.GObject.__init__(self)
      self.default_clipboard = gtk.clipboard_get()
      self.primary_clipboard = gtk.clipboard_get("PRIMARY")
      
      self.default_clipboard_text = self.default_clipboard.wait_for_text()
      self.primary_clipboard_text = self.primary_clipboard.wait_for_text()
      self.default_clipboard.connect('owner-change', self.on_default_clipboard_owner_change)
      self.primary_clipboard.connect('owner-change', self.on_primary_clipboard_owner_change)
      
      self.use_default_clipboard = glipper.GCONF_CLIENT.get_bool(glipper.GCONF_USE_DEFAULT_CLIPBOARD)
      if self.use_default_clipboard == None:
         self.use_default_clipboard = True
      glipper.GCONF_CLIENT.notify_add(glipper.GCONF_USE_DEFAULT_CLIPBOARD, lambda x, y, z, a: self.on_use_default_clipboard_changed (z.value))
      
      self.use_primary_clipboard = glipper.GCONF_CLIENT.get_bool(glipper.GCONF_USE_PRIMARY_CLIPBOARD)
      if self.use_primary_clipboard == None:
         self.use_primary_clipboard = True
      glipper.GCONF_CLIENT.notify_add(glipper.GCONF_USE_PRIMARY_CLIPBOARD, lambda x, y, z, a: self.on_use_primary_clipboard_changed (z.value))
      
   def set_text(self, text):
      if self.use_default_clipboard:
         self.default_clipboard.set_text(text)
         self.default_clipboard_text = text
         
      if self.use_primary_clipboard:
         self.primary_clipboard.set_text(text)
         self.primary_clipboard_text = text
      
      self.emit('new-item', text)
      
   def get_default_clipboard_text(self):
      return self.default_clipboard_text
   
   def on_default_clipboard_owner_change(self, clipboard, event):
      if self.use_default_clipboard:
         text = clipboard.wait_for_text()

         if text != None:
            self.default_clipboard_text = text
            self.default_clipboard_image = None
            self.emit('new-item', text)
            return

         targets = clipboard.wait_for_targets()

         if targets == None:
            if self.default_clipboard_text != None:
               clipboard.set_text(self.default_clipboard_text)
            elif self.default_clipboard_image != None:
               clipboard.set_image(self.default_clipboard_image)
   
   def on_primary_clipboard_owner_change(self, clipboard, event):
      if self.use_primary_clipboard:
         text = clipboard.wait_for_text()

         if text != None:
            self.primary_clipboard_text = text
            self.primary_clipboard_image = None
            self.emit('new-item', text)
            return

         targets = clipboard.wait_for_targets()

         if targets == None:
            if self.primary_clipboard_text != None:
               clipboard.set_text(self.primary_clipboard_text)
            elif self.default_clipboard_image != None:
               clipboard.set_image(self.primary_clipboard_image)
      
   def on_use_default_clipboard_changed (self, value):
      if value is None or value.type != gconf.VALUE_BOOL:
         return
      self.use_default_clipboard = value.get_bool()
     
   def on_use_primary_clipboard_changed (self, value):
      if value is None or value.type != gconf.VALUE_BOOL:
         return
      self.use_primary_clipboard = value.get_bool()
      
clipboards = Clipboards()

def get_glipper_clipboards():
   return clipboards

