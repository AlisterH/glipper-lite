#TODO: communicate over a secure connection
#TODO: think about a password mechanism that makes sense

import threading, socket, os, os.path, glipper, time
from gettext import gettext as _

GLIPPERPORT = 10368

allConnections = []
inserting = False
running = True

def info():
   info = {"Name": _("Network"), 
      "Description": _("Connect multiple Glipper processes via network to synchronize their history"),
      "Preferences": True}
   return info

def on_new_item(on_new_item):
   if inserting:
      return
   if on_new_item == '':
      return
   for sock in allConnections:
      try:
         sock.send(on_new_item)
      except socket.error:
         allConnections.remove(sock)
         sock.close()

#listens for new items from the other side of the connection:
class StringListener(threading.Thread):
   def __init__(self, socket):
      threading.Thread.__init__(self)
      self.socket = socket
      
   def run(self):
      while running:
         try:
            try:
               string = self.socket.recv(4096)
               if not string:
                  raise socket.error
            except socket.error:
               allConnections.remove(self.socket)
               self.socket.close()
               return
         finally:
            time.sleep(0.1)
            
         global inserting
         inserting = True
         glipper.add_history_item(string)
         inserting = False
      
#listens for incoming connections (like a server does):
class ServerListener(threading.Thread):
   def run(self):
      print "start listening for incoming connections!"
      s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      s.bind(('', GLIPPERPORT))
      s.setblocking(False)
      
      while running:
         try:
            try:
               s.listen(1)
               conn, addr = s.accept()
               print "connection %s accepted" % addr[0]
            except socket.error:
               continue
         finally:
            time.sleep(0.1)
            
         listener = StringListener(conn)
         listener.setDaemon(1)
         listener.start()
         allConnections.append(conn)
      print "stop listening for incoming connections!"
      
def stop():
   running = False
   
def init():
   #read configfile:
   f = confFile("r")
   password = f.getPassword()
   if f.getAcceptEnabled():
      acceptIPs = f.getAcceptIPs()
   else:
      acceptIPs = ()
   connectIPs = f.getConnectIPs()
   f.close()

   #First connect:
   for x in connectIPs:
      try:
         s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
         s.connect((x, GLIPPERPORT))
         allConnections.append(s)
         print "connected with %s" % x
      except socket.error:
         print "can\'t connect with %s" % x
         s.close()
         continue
      listener = StringListener(s)
      listener.setDaemon(1)
      listener.start()

   #Then listen:
   server = ServerListener()
   server.setDaemon(1)
   server.start()
   
def on_show_preferences(parent):
   preferences(parent).show()

#config file class:

class confFile:
   def __init__(self, mode):
      self.mode = mode

      dir = os.environ["HOME"] + "/.glipper/plugins"
      if (mode == "r") and (not os.path.exists(dir + "/network.conf")):
         self.password = ""
         self.enabled = False
         self.accept = ()
         self.connect = ()
         return
      if not os.path.exists(dir):
         os.makedirs(dir)
      self.file = open(dir + "/network.conf", mode)

      if mode == "r":
         self.password = self.file.readline()[:-1]
         self.enabled = (self.file.readline() == "True\n")
         self.accept = []
         self.connect = []
         line = self.file.readline()
         while line != "\n":
            self.accept.append(line[:-1])   
            line = self.file.readline()
         line = self.file.readline()
         while line != "":
            self.connect.append(line[:-1])   
            line = self.file.readline()

   def setPassword(self, password):
      self.password = password
   def getPassword(self):
      return self.password
   def setAcceptEnabled(self, enabled):
      self.enabled = enabled
   def getAcceptEnabled(self):
      return self.enabled
   def setAcceptIPs(self, ips):
      self.accept = ips
   def getAcceptIPs(self):
      return self.accept
   def setConnectIPs(self, ips):
      self.connect = ips
   def getConnectIPs(self):
      return self.connect
   def close(self):
      if not 'file' in dir(self):
         return
      try:
         if self.mode == "w":
            self.file.write(self.password + "\n")
            self.file.write(repr(self.enabled) + "\n")
            for x in self.accept:
               self.file.write(x + "\n")
            self.file.write("\n")
            for x in self.connect:
               self.file.write(x + "\n")
      finally:
         self.file.close()

#preferences dialog:

import pygtk
import gtk
import gtk.glade

class preferences:
   def __init__(self, parent):
      gladeFile = gtk.glade.XML(os.path.dirname(__file__) + "/network.glade")
      self.prefWind = gladeFile.get_widget("preferences")
      self.prefWind.set_transient_for(parent)
      self.prefWind.connect('response', self.on_prefWind_response)
      self.passEntry = gladeFile.get_widget("passEntry")
      self.accCheckbutton = gladeFile.get_widget("accCheckbutton")

      #Acception List:
      self.acceptList = gladeFile.get_widget("acceptList")
      self.acceptStore = gtk.ListStore(str)
      self.acceptList.set_model(self.acceptStore)
      renderer = gtk.CellRendererText()
      column = gtk.TreeViewColumn(None, renderer, text=0)
      self.acceptList.append_column(column) 

      #Connection List:
      self.connectList = gladeFile.get_widget("connectList")
      self.connectStore = gtk.ListStore(str)
      self.connectList.set_model(self.connectStore)
      renderer = gtk.CellRendererText()
      column = gtk.TreeViewColumn(None, renderer, text=0)
      self.connectList.append_column(column) 

      gladeFile.signal_autoconnect(self)

      #read configurations
      f = confFile("r")
      self.passEntry.set_text(f.getPassword())
      self.accCheckbutton.set_active(f.getAcceptEnabled())
      self.acceptList.set_sensitive(f.getAcceptEnabled())
      self.setStringListToStore(self.acceptStore, f.getAcceptIPs())
      self.setStringListToStore(self.connectStore, f.getConnectIPs())
      f.close()

   def show(self):
      self.prefWind.show_all()

   def askIP(self):
      dialog = gtk.Dialog(_("New connection"), self.prefWind, 
            gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
            (gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
             gtk.STOCK_OK, gtk.RESPONSE_ACCEPT))
      dialog.set_default_size (350, 150)
      info = gtk.Label(_("Enter IP address: "))
      dialog.vbox.pack_start(info)
      info.show()
      entry = gtk.Entry()
      dialog.vbox.pack_start(entry)
      entry.show()
      res = dialog.run()
      dialog.destroy()
      if (res == gtk.RESPONSE_REJECT) or (entry.get_text() == ""):
         return None
      if res == gtk.RESPONSE_ACCEPT:
         return entry.get_text()

   def addIPToList(self, store):
      IP = self.askIP()
      if IP != None:
         iter = store.append()
         store.set(iter, 0, IP)

   def removeIPFromList(self, list):
      selection = list.get_selection()
      model, iter = selection.get_selected()
      if iter != None:
         model.remove(iter)

   def getStringListFromStore(self, store):
      iter = store.get_iter_first()
      while iter != None:
         yield store.get_value(iter, 0)
         iter = store.iter_next(iter)

   def setStringListToStore(self, store, list):
      for x in list:
         iter = store.append()
         store.set(iter, 0, x)

   #EVENTS:
   def on_accCheckbutton_toggled(self, widget):
      self.acceptList.set_sensitive(widget.get_active())

   def on_delAccButton_clicked(self, widget):
      self.removeIPFromList(self.acceptList)

   def on_addAccButton_clicked(self, widget):
      self.addIPToList(self.acceptStore)

   def on_delConnButton_clicked(self, widget):
      self.removeIPFromList(self.connectList)

   def on_addConnButton_clicked(self, widget):
      self.addIPToList(self.connectStore)

   def on_prefWind_response(self, widget, response):
      if response == gtk.RESPONSE_DELETE_EVENT or response == gtk.RESPONSE_CLOSE:
         f = confFile("w")
         f.setPassword(self.passEntry.get_text())
         f.setAcceptEnabled(self.accCheckbutton.get_active())
         f.setAcceptIPs(self.getStringListFromStore(self.acceptStore))
         f.setConnectIPs(self.getStringListFromStore(self.connectStore))
         f.close()
         widget.destroy()   


