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

from glipper import *
from gettext import gettext as _

def on_new_item(item):
   try:
      if item.find(get_history_item(1)) != -1:
         remove_history_item(1)
   except:
      pass

def info():
   info = {"Name": _("Grow"),
      "Description": _("This plugin detects whether a new entry is just a grown version of the previous one, and if so, deletes the previous."),
      "Preferences": False}
   return info
