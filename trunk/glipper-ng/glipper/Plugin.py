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

import sys

class Plugin(object):
   def __init__(self, file_name):
      self.file_name = file_name
      self.load_module()
      self.info = self.call('info')
   
   def load_module(self):
      try:
         self.module = __import__(self.file_name)
      except ImportError:
         return
   
   def remove_module(self):
      del sys.modules[self.file_name]
   
   def get_file_name(self):
      return self.file_name
   
   def get_name(self):
      return self.info['Name']
      
   def get_description(self):
      return self.info['Description']
      
   def get_preferences(self):
      return self.info['Preferences']
      
   def call(self, name, *args):
      if hasattr(self.module, name):
         func = getattr(self.module, name)
         if callable(func):
            return apply(func, args)
