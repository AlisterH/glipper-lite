import gtk, gobject, glipper, gconf

class Clipboards(gobject.GObject):
   __gsignals__ = {
      "new-item" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, [gobject.TYPE_STRING]),
   }  
   
   def __init__(self):
      gobject.GObject.__init__(self)
      self.default_clipboard = gtk.clipboard_get()
      self.primary_clipboard = gtk.clipboard_get("PRIMARY")
      
      self.previous_primary_clipboard_text = self.primary_clipboard.wait_for_text()
      self.previous_default_clipboard_text = self.default_clipboard.wait_for_text()
      self.use_default_clipboard = glipper.GCONF_CLIENT.get_bool(glipper.GCONF_USE_DEFAULT_CLIPBOARD)
      if self.use_default_clipboard == None:
         self.use_default_clipboard = True
      glipper.GCONF_CLIENT.notify_add(glipper.GCONF_USE_DEFAULT_CLIPBOARD, lambda x, y, z, a: self.on_use_default_clipboard_changed (z.value))
      
      self.use_primary_clipboard = glipper.GCONF_CLIENT.get_bool(glipper.GCONF_USE_PRIMARY_CLIPBOARD)
      if self.use_primary_clipboard == None:
         self.use_primary_clipboard = True
      glipper.GCONF_CLIENT.notify_add(glipper.GCONF_USE_PRIMARY_CLIPBOARD, lambda x, y, z, a: self.on_use_primary_clipboard_changed (z.value))
      
      gobject.timeout_add(500, self.on_timeout)
   
   def set_default_clipboard_text(self, text):
      self.default_clipboard.set_text(text)
   
   def get_default_clipboard_text(self):
      return self.previous_default_clipboard_text
   
   def on_timeout(self):
      if self.use_default_clipboard:
         item = self.default_clipboard.wait_for_text()
         if item != None and item != self.previous_default_clipboard_text:
            self.previous_default_clipboard_text = item
            self.emit('new-item', item)
            
      if self.use_primary_clipboard:
         item = self.primary_clipboard.wait_for_text()
         if item != None and item != self.previous_primary_clipboard_text:
            self.previous_primary_clipboard_text = item
            self.emit('new-item', item)
      
      return True
            
   def on_use_default_clipboard_changed (self, value):
      if value is None or value.type != gconf.VALUE_BOOL:
         return
      self.use_default_clipboard = value.get_bool()
     
   def on_use_primary_clipboard_changed (self, value):
      if value is None or value.type != gconf.VALUE_BOOL:
         return
      self.use_primary_clipboard = value.get_bool()
