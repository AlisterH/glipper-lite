import glipper, os, os.path

static_items = []

def loadStaticItems():
    dir = os.environ["HOME"] + "/.glipper/plugins"
    if not os.path.exists(dir):
        os.makedirs(dir)
    try: file = open(dir + "/staticitems", "r")
    except IOError:
        return
    else:
        length = file.readline()[:-1]
        while(length):
            static_items.append(file.read(int(length)+1)[:-1])
            length = file.readline()[:-1]
        file.close()

def saveStaticItems():
    dir = os.environ["HOME"] + "/.glipper/plugins"
    if not os.path.exists(dir):
        os.makedirs(dir)
    file = open(dir + "/staticitems", "w")
    for item in static_items:
        file.write(str(len(item)))
        file.write("\n" + item + "\n")
    file.close()

import pygtk
pygtk.require('2.0')
import gtk
import gtk.glade

class manager:

    def updateHistoryModel(self):
        try: self.historyModel
        except AttributeError:
            return
        else:
            self.historyModel.clear()
            index = 0
            item = glipper.getItem(index)
            while(item):
                self.historyModel.append ([item])
                index = index + 1
                item = glipper.getItem(index)

    def create(self):
        gladeFile = gtk.glade.XML(os.path.dirname(__file__) + "/staticitems.glade")
        self.managerWindow = gladeFile.get_widget("manager")
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
        self.updateHistoryModel()
        for item in static_items:
            self.staticItemsModel.append([item])
        gladeFile.signal_autoconnect(self)

	# Events:

    def on_addButton_clicked(self, widget):
        self.historyModel, iter = self.historySelection.get_selected()
        if iter:
            if not static_items.__contains__(self.historyModel.get_value(iter,0)):
                static_items.append(self.historyModel.get_value(iter, 0))
                self.staticItemsModel.append([self.historyModel.get_value(iter, 0)])
                saveStaticItems()

    def on_removeButton_clicked(self, widget):
        self.staticItemsModel, iter = self.staticItemsSelection.get_selected()
        if iter:
            static_items.remove(self.staticItemsModel.get_value(iter, 0))
            self.staticItemsModel.remove(iter)
            saveStaticItems()

    def on_activateButton_clicked(self, widget):
        self.staticItemsModel, iter = self.staticItemsSelection.get_selected()
        if iter:
            glipper.insertItem(self.staticItemsModel.get_value(iter, 0))
            glipper.setActiveItem(0)

static_items_manager = manager()

def showManager():
    static_items_manager.create()

def init():
    glipper.registerEntry("Static Items", showManager)
    loadStaticItems()

def showPreferences():
    pass

def newItem(arg):
    static_items_manager.updateHistoryModel()

def clearHistory():
    static_items_manager.updateHistoryModel()

def getInfo():
	info = {"Name": "Static Items",
		"Description": "This plugin adds the option to manage static items",
		"Preferences": False}
	return info
