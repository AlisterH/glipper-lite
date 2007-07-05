import os, os.path, re
import glipper

import pygtk
pygtk.require('2.0')
import gtk
import gtk.glade
import pango

immediately = False

menu = gtk.Menu()

def on_history_changed():
   update_menu()

def update_menu():
   global immediately
   global menu
   global menu_item
   
   cf = confFile("r")
   immediately = cf.getImmediately()
   model = cf.getActionModel()
   
   menu.destroy()
   menu = gtk.Menu()
   
   #read informations from model and create menu items:

   item = glipper.get_history_item(0)
   if len(model) == 0 or item == None:
      empty_item = gtk.MenuItem('No actions available')
      menu.append(empty_item)
   else:
      for action in model:
         regex = re.compile(action[0])
         if regex.match(item) != None:
            for cmd in action.iterchildren():
               item = gtk.MenuItem(cmd[3])
               menu.append(item)
               item.connect("activate", commandActivated, cmd[0])
   
   menu.show_all()
   menu_item.set_submenu(menu)
   
   cf.close()

def commandActivated(menu, cmd):
   command = cmd.replace("%s", glipper.get_history_item(0))
   os.system(command)

menu_item = gtk.MenuItem("Actions")

def init():
   global menu_item
   update_menu()
   glipper.add_menu_item(__name__, menu_item)

def stop():
   menu.destroy()

def on_new_item(newItem):
   if immediately:
      menu.popup(None, None, None, 0, 0)

def on_show_preferences(parent):
   preferences(parent).show()

def info():
   info = {"Name": "Actions", 
      "Description": "Define commands to run when an item matches a regular expression",
      "Preferences": True}
   return info

#config file class:
class confFile:
   def __init__(self, mode):
      self.mode = mode

      dir = os.environ["HOME"] + "/.glipper/plugins"
      if (mode == "r") and (not os.path.exists(dir + "/actions.conf")):
         self.immediately = False
         self.actionModel = gtk.TreeStore(str, pango.Style, str, str, pango.Style, str)
         #Todo: provide some standard entries here
         return
      if not os.path.exists(dir):
         os.makedirs(dir)
      self.file = open(dir + "/actions.conf", mode)

      if mode == "r":
         self.immediately = self.file.readline()[:-1] == "True"
         self.actionModel = gtk.TreeStore(str, pango.Style, str, str, pango.Style, str)
         regex = self.file.readline()[:-1]
         while (regex):
            descr = self.file.readline()[:-1]
            iter = self.actionModel.append(None, 
               row=(regex, pango.STYLE_NORMAL, "#000", descr, pango.STYLE_NORMAL, "#000",))
            cmd = self.file.readline()[:-1]
            while (cmd and cmd[0] == "\t"):
               descr = self.file.readline()[:-1]
               self.actionModel.append(iter, 
                  row=(cmd[1:], pango.STYLE_NORMAL, "#000", 
                       descr[1:], pango.STYLE_NORMAL, "#000"))
               cmd = self.file.readline()[:-1]
            regex = cmd

   def setImmediately(self, im):
      self.immediately = im
   def getImmediately(self):
      return self.immediately
   def setActionModel(self, model):
      self.actionModel = model
   def getActionModel(self):
      return self.actionModel
   def close(self):
      if not 'file' in dir(self):
         return
      try:
         if self.mode == "w":
            self.file.write(str(self.immediately) + "\n")
            for regex in self.actionModel:
               if regex[1] == pango.STYLE_NORMAL:
                  self.file.write(regex[0] + "\n")
                  self.file.write(regex[3] + "\n")
                  for cmd in regex.iterchildren():
                     if cmd[1] == pango.STYLE_NORMAL:
                        self.file.write("\t" + cmd[0] + "\n")
                        self.file.write("\t" + cmd[3] + "\n")
      finally:
         self.file.close()

#preferences dialog:

class preferences:
   def __init__(self, parent):
      gladeFile = gtk.glade.XML(os.path.dirname(__file__) + "/actions.glade")
      self.prefWind = gladeFile.get_widget("preferences")
      self.prefWind.set_transient_for(parent)
      self.prefWind.connect('response', self.on_prefWind_response)
      self.addCommandButton = gladeFile.get_widget("addCommandButton")
      self.addCommandButton.set_sensitive(False);
      self.immediatelyCheck = gladeFile.get_widget("immediatelyCheck")
      self.actionTree = gladeFile.get_widget("actionTree")

      gladeFile.signal_autoconnect(self)

      #read configurations
      f = confFile("r")
      self.immediatelyCheck.set_active(f.getImmediately())
      self.actionModel = f.getActionModel()
      self.actionTree.set_model(self.actionModel)
      f.close()

      #setup TreeView:
      cellRenderer = gtk.CellRendererText()
      cellRenderer.set_property("editable", True)
      cellRenderer.connect("edited", self.cellEdited, 0)
      column = gtk.TreeViewColumn("Action", cellRenderer, text=0, style=1, foreground=2)
      column.set_resizable(True)
      self.actionTree.append_column(column)

      cellRenderer = gtk.CellRendererText()
      cellRenderer.set_property("editable", True)
      cellRenderer.connect("edited", self.cellEdited, 3)
      column = gtk.TreeViewColumn("Description", cellRenderer, text=3, style=4, foreground=5)
      column.set_resizable(True)
      self.actionTree.append_column(column)
      self.actionTree.get_selection().set_mode(gtk.SELECTION_SINGLE)

      self.actionTree.get_selection().connect("changed", self.selectionChanged)

      self.menu1 = gtk.Menu()
      item = gtk.MenuItem("Add command")
      item.connect("activate", self.addCommand)
      self.menu1.append(item)
      item = gtk.MenuItem("Delete action")
      item.connect("activate", self.deleteEntry)
      self.menu1.append(item)
      self.menu1.show_all()

      self.menu2 = gtk.Menu()
      item = gtk.MenuItem("Delete command")
      item.connect("activate", self.deleteEntry)
      self.menu2.append(item)
      self.menu2.show_all()

   def show(self):
      self.prefWind.show_all()
   
   def addCommand(self, menu):
      iter = self.actionTree.get_selection().get_selected()[1]
      self.actionModel.append(iter, 
         row=("New command", pango.STYLE_ITALIC, "#666", 
            "Enter description here", pango.STYLE_ITALIC, "#666"))
      self.actionTree.expand_row(self.actionModel.get_path(iter), False)

   def deleteEntry(self, menu):
      iter = self.actionTree.get_selection().get_selected()[1]
      self.actionModel.remove(iter)

   #EVENTS:
   def selectionChanged(self, selection):
      self.addCommandButton.set_sensitive(selection.get_selected()[1] != None)

   def cellEdited(self, renderer, path, new_text, col):
      iter = self.actionModel.get_iter(path)
      if self.actionModel.get_value(iter, col) != new_text:
         self.actionModel.set_value(iter, col, new_text)
         self.actionModel.set_value(iter, col+1, pango.STYLE_NORMAL)
         self.actionModel.set_value(iter, col+2, "#000")

   def on_prefWind_response(self, widget, response):
      if response == gtk.RESPONSE_DELETE_EVENT or response == gtk.RESPONSE_CLOSE:
         f = confFile("w")
         f.setImmediately(self.immediatelyCheck.get_active())
         f.setActionModel(self.actionModel)
         f.close()
         widget.destroy()
         update_menu()
            
   def addButton_clicked(self, widget):
      self.actionModel.append(None, 
         row=("New regular expression", pango.STYLE_ITALIC, "#666", 
            "Enter description here", pango.STYLE_ITALIC, "#666"))

   def deleteButton_clicked(self, widget):
      iter = self.actionTree.get_selection().get_selected()[1]
      self.actionModel.remove(iter)

   def addCommandButton_clicked(self, widget):
      iter = self.actionTree.get_selection().get_selected()[1]
      if self.actionModel.iter_depth(iter) == 1:
         iter = self.actionModel.iter_parent(iter)
      self.actionModel.append(iter, 
         row=("New command", pango.STYLE_ITALIC, "#666", 
            "Enter description here", pango.STYLE_ITALIC, "#666"))
      self.actionTree.expand_row(self.actionModel.get_path(iter), False)

   def actionTree_button_press_event_cb(self, widget, event):
      if event.button == 3:
         #do we really have to do this by hand: (???)
         selection = self.actionTree.get_selection()
         selection.unselect_all()
         (x, y) = event.get_coords()
         x = int(x); y = int(y)
         if not self.actionTree.get_path_at_pos(x, y): return
         (path, col, x, y) = self.actionTree.get_path_at_pos(x, y)
         selection.select_path(path)
         iter = self.actionModel.get_iter(path)

         if self.actionModel.iter_depth(iter) == 0:
            self.menu1.popup(None, None, None, event.button, event.time)
         elif self.actionModel.iter_depth(iter) == 1:
            self.menu2.popup(None, None, None, event.button, event.time)
      return False
      
