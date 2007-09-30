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

import glipper, os, os.path, gconf

from gettext import gettext as _

snippets = []

def load_snippets():
    global snippets
    snippets = []
    dir = os.environ["HOME"] + "/.glipper/plugins"
    if not os.path.exists(dir):
        os.makedirs(dir)
    try: file = open(dir + "/snippets", "r")
    except IOError:
        return
    else:
        length = file.readline()[:-1]
        while(length):
            snippets.append(file.read(int(length)+1)[:-1])
            length = file.readline()[:-1]
        file.close()

def save_snippets():
    dir = os.environ["HOME"] + "/.glipper/plugins"
    if not os.path.exists(dir):
        os.makedirs(dir)
    file = open(dir + "/snippets", "w")
    for item in snippets:
        file.write(str(len(item)))
        file.write("\n" + item + "\n")
    file.close()


import gtk
import gtk.glade

class Manager:
    def update_history_model(self):
      self.history_model.clear()
      index = 0
      item = glipper.get_history_item(index)
      while(item):
         self.history_model.append ([item])
         index = index + 1
         item = glipper.get_history_item(index)
      
    def __init__(self, parent):
        glade_file = gtk.glade.XML(os.path.dirname(__file__) + "/snippets.glade")
        self.manager = glade_file.get_widget("manager")
        self.manager.show_all()
        self.history_model = gtk.ListStore (str)
        self.history_tree = glade_file.get_widget('history_tree')
        self.history_tree.set_model(self.history_model)
        self.history_text = gtk.CellRendererText ()
        self.history_column = gtk.TreeViewColumn (_("History"), self.history_text, text = 0)
        self.history_tree.append_column (self.history_column)
        self.history_selection = self.history_tree.get_selection()
        self.history_selection.set_mode (gtk.SELECTION_SINGLE)
        self.snippets_model = gtk.ListStore (str)
        self.snippets_tree = glade_file.get_widget('snippets_tree')
        self.snippets_tree.set_model(self.snippets_model)
        self.snippets_text = gtk.CellRendererText ()
        self.snippets_column = gtk.TreeViewColumn (_("Snippets"), self.snippets_text, text = 0)
        self.snippets_tree.append_column (self.snippets_column)
        self.snippets_selection = self.snippets_tree.get_selection()
        self.snippets_selection.set_mode (gtk.SELECTION_SINGLE)
        self.update_history_model()
        
        for item in snippets:
            self.snippets_model.append([item])
        
        self.manager.set_transient_for(parent)
        
        glade_file.signal_autoconnect(self)

    def on_manager_response(self, window, response):
      if response == gtk.RESPONSE_DELETE_EVENT or response == gtk.RESPONSE_CLOSE:
         window.destroy()
      if response == 1:
         self.update_history_model()
	
    def on_add_button_clicked(self, widget):
        self.history_model, iter = self.history_selection.get_selected()
        if iter:
            if self.history_model.get_value(iter,0) not in snippets:
                snippets.append(self.history_model.get_value(iter, 0))
                self.snippets_model.append([self.history_model.get_value(iter, 0)])
                save_snippets()
                update_menu()

    def on_remove_button_clicked(self, widget):
        self.snippets_model, iter = self.snippets_selection.get_selected()
        if iter:
            snippets.remove(self.snippets_model.get_value(iter, 0))
            self.snippets_model.remove(iter)
            save_snippets()
            update_menu()

def on_show_preferences(parent):
    load_snippets()
    Manager(parent)

menu_item = gtk.MenuItem(_("Snippets"))
menu = gtk.Menu()
tooltips = gtk.Tooltips()

def init():
    load_snippets()
    glipper.add_menu_item(menu_item)
    glipper.GCONF_CLIENT.notify_add(glipper.GCONF_MAX_ITEM_LENGTH, lambda x, y, z, a: update_menu())
    update_menu()

def on_activate(menuitem, snippet):
   glipper.add_history_item(snippet)

def update_menu():
   max_length = glipper.GCONF_CLIENT.get_int(glipper.GCONF_MAX_ITEM_LENGTH)
   global menu
   menu.destroy()
   menu = gtk.Menu()
   
   if len(snippets) == 0:
      menu.append(gtk.MenuItem(_("No snippets available")))
   else:
      for snippet in snippets:
         item = gtk.MenuItem(glipper.format_item(snippet))
         if len(snippet) > max_length:
            tooltips.set_tip(item, snippet)
         item.connect('activate', on_activate, snippet)
         menu.append(item)
   
   menu.show_all()
   menu_item.set_submenu(menu)

def stop():
   menu.destroy()

def on_history_changed():
    update_menu()

def info():
	info = {"Name": _("Snippets"),
		"Description": _("Create snippets from history items"),
		"Preferences": True }
	return info
