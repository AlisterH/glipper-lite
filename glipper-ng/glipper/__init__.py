import os, sys
from os.path import join, exists, isdir, isfile, dirname, abspath, expanduser

import gtk, gtk.gdk, gconf

# Autotools set the actual data_dir in defs.py
from defs import *

try:
   # Allows to load uninstalled .la libs
   import ltihooks
except ImportError:
   pass

# Allow to use uninstalled glipper ---------------------------------------------
UNINSTALLED_GLIPPER = False
def _check(path):
   return exists(path) and isdir(path) and isfile(path + "/AUTHORS")
   
name = join(dirname(__file__), '..')
if _check(name):
   UNINSTALLED_GLIPPER = True
   
# Sets SHARED_DATA_DIR to local copy, or the system location
# Shared data dir is most the time /usr/share/glipper
if UNINSTALLED_GLIPPER:
   SHARED_DATA_DIR = abspath(join(dirname(__file__), '..', 'data'))
else:
   SHARED_DATA_DIR = join(DATA_DIR, "glipper")
print "SHARED_DATA_DIR: %s" % SHARED_DATA_DIR


USER_GLIPPER_DIR = expanduser("~/.glipper")
if not exists(USER_GLIPPER_DIR):
   try:
      os.makedirs(USER_GLIPPER_DIR, 0744)
   except Exception , msg:
      print 'Error:could not create user glipper dir (%s): %s' % (USER_GLIPPER_DIR, msg)

# ------------------------------------------------------------------------------

# Set the cwd to the home directory so spawned processes behave correctly
# when presenting save/open dialogs
os.chdir(expanduser("~"))

# Path to plugins
PLUGINS_DIR = join(SHARED_DATA_DIR, 'plugins')

# Path to history file
HISTORY_FILE = join(USER_GLIPPER_DIR, "history")

#Gconf client
GCONF_CLIENT = gconf.client_get_default()

# GConf directory for deskbar in window mode and shared settings
GCONF_DIR = "/apps/glipper"

# GConf key to the setting for the amount of elements in history
GCONF_MAX_ELEMENTS = GCONF_DIR + "/max_elements"

# GConf key to the setting for the length of one history item
GCONF_MAX_ITEM_LENGTH = GCONF_DIR + "/max_item_length"

# GConf key to the setting for the key combination to popup glipper
GCONF_KEY_COMBINATION = GCONF_DIR + "/key_combination"

# GConf key to the setting for using the default clipboard
GCONF_USE_DEFAULT_CLIPBOARD = GCONF_DIR + "/use_default_clipboard"

# GConf key to the setting for using the primary clipboard
GCONF_USE_PRIMARY_CLIPBOARD = GCONF_DIR + "/use_primary_clipboard"

# GConf key to the setting for whether the default entry should be marked in bold
GCONF_MARK_DEFAULT_ENTRY = GCONF_DIR + "/mark_default_entry"

# GConf key to the setting for whether the history should be saved
GCONF_SAVE_HISTORY = GCONF_DIR + "/save_history"

GCONF_AUTOSTART_PLUGINS = GCONF_DIR + "/autostart_plugins"

# Preload gconf directories
GCONF_CLIENT.add_dir(GCONF_DIR, gconf.CLIENT_PRELOAD_RECURSIVE)

# Functions callable by plugins

from glipper.PluginsManager import *
from glipper.History import *
from glipper.Clipboards import *
import glipper.Applet

def add_menu_item(file_name, menu_item):
   get_glipper_plugins_manager().add_menu_item(file_name, menu_item)

def add_history_item(item):
   get_glipper_clipboards().set_text(item)

def set_history_item(index, item):
   get_glipper_history().set(index, item)

def get_history_item(index):
   return get_glipper_history().get(index)

def format_item(item):
   return glipper.Applet.format_item(item)
