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
	return exists(path) and isdir(path) and isfile(path+"/AUTHORS")
	
name = join(dirname(__file__), '..')
if _check(name):
	UNINSTALLED_GLIPPER = True
	
if UNINSTALLED_GLIPPER:
	SHARED_DATA_DIR = abspath(join(dirname(__file__), '..', 'data'))
else:
	SHARED_DATA_DIR = join(DATA_DIR, "glipper")

USER_GLIPPER_DIR = expanduser("~/.glipper")
if not exists(USER_GLIPPER_DIR):
   try:
      os.makedirs(USER_GLIPPER_DIR, 0744)
   except Exception , msg:
      print 'Error:could not create user glipper dir (%s): %s' % (USER_GLIPPER_DIR, msg)

# Path to plugins
if UNINSTALLED_GLIPPER:
	PLUGINS_DIR = join(dirname(__file__), "plugins")
else:
	PLUGINS_DIR = join(SHARED_DATA_DIR, "plugins")

# ------------------------------------------------------------------------------

# Set the cwd to the home directory so spawned processes behave correctly
# when presenting save/open dialogs
os.chdir(expanduser("~"))

# Path to history file
HISTORY_FILE = join(USER_GLIPPER_DIR, "history")

# Maximum length constant for tooltips item in the history
MAX_TOOLTIPS_LENGTH = 11347

#Gconf client
GCONF_CLIENT = gconf.client_get_default()

# GConf directory for glipper in window mode and shared settings
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

from glipper.History import *
from glipper.Clipboards import *
from glipper.PluginsManager import *

import glipper.Applet

def add_menu_item(menu_item):
   get_glipper_plugins_manager().add_menu_item(menu_item)

def add_history_item(item):
   get_glipper_clipboards().set_text(item)

def set_history_item(index, item):
   get_glipper_history().set(index, item)

def get_history_item(index):
   return get_glipper_history().get(index)

def remove_history_item(index):
   return get_glipper_history().remove(index)

def clear_history():
   return get_glipper_history().clear()

def format_item(item):
   return glipper.Applet.format_item(item)
