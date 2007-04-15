import os, os.path, webbrowser
import glipper

import pygtk
pygtk.require('2.0')
import gtk
import gtk.glade
import pango

def getInfo():
	info = {"Name": "Actions", 
		"Description": "Let\'s you define actions for new items",
		"Preferences": True}
	return info

def activated():
	cf = confFile("r")
	cf.close()

def init():
	glipper.registerEntry("Actions", activated)

def newItem(newItem):
	pass

def showPreferences():
	preferences().show()


#config file class:
class confFile:
	def __init__(self, mode):
		self.mode = mode

		dir = os.environ["HOME"] + "/.glipper/plugins"
		if (mode == "r") and (not os.path.exists(dir + "/actions.conf")):
			self.immediately = False
			self.actionModel = gtk.TreeStore(str, str)
			return
		if not os.path.exists(dir):
			os.makedirs(dir)
		self.file = open(dir + "/actions.conf", mode)

		if mode == "r":
			self.immediately = bool(self.file.readline()[:-1])
			self.actionModel = gtk.TreeStore(str, str)
			regex = self.file.readline()[:-1]
			while (regex):
				descr = self.file.readline()[:-1]
				iter = self.actionModel.append(None, row=(regex, descr))
				cmd = self.file.readline()[:-1]
				while (cmd and cmd[0] == "\t"):
					descr = self.file.readline()[:-1]
					self.actionModel.append(iter, row=(cmd[1:], descr[1:]))
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
					self.file.write(regex[0] + "\n")
					self.file.write(regex[1] + "\n")
					for cmd in regex.iterchildren():
						self.file.write("\t" + cmd[0] + "\n")
						self.file.write("\t" + cmd[1] + "\n")
		finally:
			self.file.close()

#preferences dialog:

class preferences:
	def __init__(self):
		gladeFile = gtk.glade.XML(os.path.dirname(__file__) + "/actions.glade")
		self.prefWind = gladeFile.get_widget("preferences")
		self.prefWind.connect("destroy", self.destroy)
		self.prefWind.show()
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
		cellRenderer.set_property("style", pango.STYLE_ITALIC)
		cellRenderer.set_property("foreground", "#666")
		column = gtk.TreeViewColumn("Action", cellRenderer, text=0)
		column.set_resizable(True)
		#column.set_sizing(gtk.TREE_VIEW_COLUMN_AUTOSIZE)
		self.actionTree.append_column(column)

		cellRenderer = gtk.CellRendererText()
		cellRenderer.set_property("editable", True)
		cellRenderer.connect("edited", self.cellEdited, 1)
		cellRenderer.set_property("style", pango.STYLE_ITALIC)
		cellRenderer.set_property("foreground", "#666")
		column = gtk.TreeViewColumn("Description", cellRenderer, text=1)
		column.set_resizable(True)
		#column.set_sizing(gtk.TREE_VIEW_COLUMN_AUTOSIZE)
		self.actionTree.append_column(column)
		self.actionTree.get_selection().set_mode(gtk.SELECTION_SINGLE)

		self.menu1 = gtk.Menu()
		item = gtk.MenuItem("add command")
		item.connect("activate", self.addCommand)
		self.menu1.append(item)
		item = gtk.MenuItem("delete action")
		item.connect("activate", self.deleteEntry)
		self.menu1.append(item)
		self.menu1.show_all()

		self.menu2 = gtk.Menu()
		item = gtk.MenuItem("delete command")
		item.connect("activate", self.deleteEntry)
		self.menu2.append(item)
		self.menu2.show_all()

	def show(self):
		gtk.main()
	
	def addCommand(self, menu):
		iter = self.actionTree.get_selection().get_selected()[1]
		self.actionModel.append(iter, row=("command", "Description"))
		self.actionTree.expand_row(self.actionModel.get_path(iter), False)

	def deleteEntry(self, menu):
		iter = self.actionTree.get_selection().get_selected()[1]
		self.actionModel.remove(iter)

	#EVENTS:
	def cellEdited(self, renderer, path, new_text, col):
		iter = self.actionModel.get_iter(path)
		self.actionModel.set_value(iter, col, new_text)

	def on_applyButton_clicked(self, widget):
		f = confFile("w")
		f.setImmediately(self.immediatelyCheck.get_active())
		f.setActionModel(self.actionModel)
		f.close()
		self.prefWind.destroy()	
				
	def addButton_clicked(self, widget):
		self.actionModel.append(None, row=("New regular expression", "Enter description here"))

	def deleteButton_clicked(self, widget):
		iter = self.actionTree.get_selection().get_selected()[1]
		self.actionModel.remove(iter)

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

	def destroy(self, widget):
		gtk.main_quit()
		
