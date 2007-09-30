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

#This is a small example plugin, which just adds a newline character at the end of every new clipboardsentry line
#
#Please be aware that this plugin doesn't work very well, due to the very strange implementation of clipboard
#functionality by X11.
#You can see this for example, if you mark something on the console. If you have this plugin enabled,
#the selection will disapear imediately.
#This is because of the fact that we change the content of the clipboard immediately, and so glipper
#becomes the owner of the clipboard. The console application you are using lost his ownership, and so
#the selection disappears

import glipper
from gettext import gettext as _

recursive = False
def on_new_item(arg):
   global recursive
   if recursive:
      recursive = False
   else:
      recursive = True
      i = arg + '\n'
      glipper.set_history_item(0, i)
      glipper.add_history_item(i)
   
def info():
   info = {"Name": _("New line"),
      "Description": _("Example plugin that adds a newline character at the end of items in the clipboard"),
      "Preferences": False}
   return info
