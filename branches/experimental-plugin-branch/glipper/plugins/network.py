import threading, socket, glipper

GLIPPERPORT = 10368

allConnections = []

def getInfo():
	info = {"Name": "network connection", 
		"Description": "Let\'s you connect multiple glipper processes via network to synchronize their history",
		"Preferences": True}
	return info

def newItem(newItem):
	if newItem == '':
		return
	for sock in allConnections:
		try:
			sock.send(newItem)
		except socket.error:
			allConnections.remove(sock)
			sock.close()

#listens for new items from the other side of the connection:
class StringListener(threading.Thread):
	def __init__(self, socket):
		threading.__init__(self)
		self.socket = socket
	def run(self):
		while 1:
			try:
				string = self.socket.recv(4096)
			except socket.error:
				allConnections.remove(self.socket)
				self.socket.close()
				return
			glipper.insertItem(string)

#listens for incoming connections (like a server does):
class ServerListener(threading.Thread):
	def run(self):
		print "start listening for incoming connections!"
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.bind(('', GLIPPERPORT))
		while 1:
			try:
				s.listen(1)
				conn, addr = s.accept()
				print "connection %i accepted" % addr
			except socket.error:
				continue
			StringListener(conn).start()
			allConnections.append(conn)
		

connectWith = ('127.0.0.1')

def init():
	print "Test"
	#First connect:
	for x in connectWith:
		try:
			s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			s.connect((x, GLIPPERPORT))
			allConnections.append(s)
			print "connected with %i" % x
		except socket.error:
			print "can\'t connect with %i" % x
			s.close()
			continue
		StringListener(s).start()

	#Then listen:
	ServerListener().start()


import pygtk
pygtk.require('2.0')
import gtk

def showPreferences():
	window = gtk.Window(gtk.WINDOW_TOPLEVEL)
	mainVBox = gtk.VBox()
	ApplyButton = gtk.Button(stock=gtk.STOCK_APPLY)
	mainVBox.pack_start(ApplyButton)
	window.add(mainVBox)
	window.show()
	gtk.main()
