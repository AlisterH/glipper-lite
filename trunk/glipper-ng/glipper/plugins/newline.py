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
