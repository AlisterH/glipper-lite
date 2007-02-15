import glipper, os, os.path

import pygtk
pygtk.require('2.0')
import gtk
import gtk.glade

static_items = []

def loadStaticItems():
    savedFile = open(os.environ["HOME"] + "/.glipper/plugins/staticitems", "r")
    length = savedFile.readline()[:-1]
    while(length):
        static_items.append(savedFile.read(int(length)+1)[:-1])
        length = savedFile.readline()[:-1]
    savedFile.close()

def saveStaticItems():
    savedFile = open(os.environ["HOME"] + "/.glipper/plugins/staticitems", "w")
    for item in static_items:
        savedFile.write(str(len(item)))
        savedFile.write("\n" + item + "\n")
    savedFile.close()

class manager:
    def updateModels(self):
        self.historyModel.clear()
        self.staticItemsModel.clear()
        index = 0
        item = glipper.getItem(index)
        while(item):
            if not static_items.__contains__(item):
                self.historyModel.append ([item])
            index = index + 1
            item = glipper.getItem(index)
        for item in static_items:
            self.staticItemsModel.append ([item])

    def __init__(self):
        gladeFile = gtk.glade.XML(os.path.dirname(__file__) + "/staticitems.glade")
        self.managerWindow = gladeFile.get_widget("manager")
        self.managerWindow.connect("destroy", self.destroy)
        self.managerWindow.show()
        self.historyModel = gtk.ListStore (str)
        self.historyTree = gladeFile.get_widget('historyTree')
        self.historyTree.set_model(self.historyModel)
        self.historyText = gtk.CellRendererText ()
        self.historyColumn = gtk.TreeViewColumn (None, self.historyText, text = 0)
        self.historyTree.append_column (self.historyColumn)
        self.historySelection = self.historyTree.get_selection()
        self.historySelection.set_mode (gtk.SELECTION_SINGLE)
        self.staticItemsModel = gtk.ListStore (str)
        self.staticItemsTree = gladeFile.get_widget('staticItemsTree')
        self.staticItemsTree.set_model(self.staticItemsModel)
        self.staticItemsText = gtk.CellRendererText ()
        self.staticItemsColumn = gtk.TreeViewColumn (None, self.staticItemsText, text = 0)
        self.staticItemsTree.append_column (self.staticItemsColumn)
        self.staticItemsSelection = self.staticItemsTree.get_selection()
        self.staticItemsSelection.set_mode (gtk.SELECTION_SINGLE)
        self.updateModels()
        gladeFile.signal_autoconnect(self)

    def show(self):
        gtk.main()

    def destroy(self, widget):
        gtk.main_quit()

	#EVENTS:
    def on_addButton_clicked(self, widget):
        self.historyModel, iter = self.historySelection.get_selected()
        if iter:
            static_items.append(self.historyModel.get_value(iter, 0))
            self.updateModels()
            saveStaticItems()

    def on_removeButton_clicked(self, widget):
        self.staticItemsModel, iter = self.staticItemsSelection.get_selected()
        if iter:
            static_items.remove(self.staticItemsModel.get_value(iter, 0))
            self.updateModels()
            saveStaticItems()

def showManager():
    manager().show()

def init():
    glipper.registerEntry("Static Items Manager", showManager)
    loadStaticItems()
    for item in static_items:
        glipper.insertItem(item, True)

def stop():
    index = 0
    save_items = []
    item = glipper.getItem(index)
    while(item):
        if not static_items.__contains__(item):
            save_items.append(item)
        index = index + 1
        item = glipper.getItem(index)
    glipper.clearHistory()
    for item in save_items:
        glipper.insertItem(item, True)

def afterDeleteList():
    for item in static_items:
        glipper.insertItem(item, True)

def getInfo():
	info = {"Name": "Static Items",
		"Description": "This plugin adds the option to manage static items",
		"Preferences": False}
	return info
