import gtk, gobject, glipper, gconf

class Clipboards(gobject.GObject):
   
   __gsignals__ = {
      "new-item" : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, [gobject.TYPE_STRING]),
   }  
   
   def __init__(self):
      gobject.GObject.__init__(self)
      self.default_clipboard = gtk.clipboard_get()
      self.primary_clipboard = gtk.clipboard_get("PRIMARY")
      
      self.default_clipboard_text = None
      self.primary_clipboard_text = None
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
         item = clipboard.wait_for_text()
         
         if item != None:
            if item != self.default_clipboard_text:
               self.default_clipboard_text = item
               self.emit('new-item', item)
         elif self.default_clipboard_text != None:
            clipboard.set_text(self.default_clipboard_text)
   
   def on_primary_clipboard_owner_change(self, clipboard, event):
      if self.use_primary_clipboard:
         item = clipboard.wait_for_text()
         
         if item != None:
            if item != self.primary_clipboard_text:
               self.primary_clipboard_text = item
               self.emit('new-item', item)
         elif self.primary_clipboard_text != None:
            clipboard.set_text(self.primary_clipboard_text)
      
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

