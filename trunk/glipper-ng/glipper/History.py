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
      get_glipper_plugins_manager().call('newItem', item)
         
   def clear(self):
      self.history = []
      self.emit('changed', self.history)
   
   def get(self, index):
      try:
         return self.history[index]
      except IndexError:
         return
   
   def set(self, index, item):
      if item not in self.history:
         try:
            self.history[index] = item
         except IndexError:
            return
            
         self.emit('changed', self.history)
   
   def add(self, item):
      if item not in self.history:
         self.history.insert(0, item)
         if len(self.history) > self.max_elements:
            self.history = self.history[0:self.max_elements]
         self.emit('changed', self.history)
   
   def set_default(self, item):
      self.add(item)
      get_glipper_clipboards().set_default_clipboard_text(item)
      self.emit('changed', self.history)
   
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
