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
