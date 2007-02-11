import httplib, urllib, os, os.path, webbrowser
import glipper

def getInfo():
	info = {"Name": "No-Paste", 
		"Description": "Let\'s you paste the entry of your clipboard to a No-Paste service",
		"Preferences": True}
	return info

def rafbnet(lang, nick, desc, text):
	conn = httplib.HTTPConnection("rafb.net")
	params = urllib.urlencode({"lang": lang, "nick": nick, 
		"desc": desc, "text": text, "tabs": "no"})
	headers = {"Content-type": "application/x-www-form-urlencoded", "Accept": "text/plain"}
	conn.request("POST", "/paste/paste.php", params, headers)
	url = conn.getresponse().getheader("location")
	conn.close()
	return url

def activated():
	languageList = ("C89", "C", "C++", "C#", "Java", "Pascal", "Perl", "PHP", 
			"PL/I", "Python", "Ruby", "SQL", "VB", "Plain Text")
	cf = confFile("r")
	url = rafbnet(languageList[cf.getLang()], cf.getNick(), "pasted by glipper", glipper.getItem(0))
	webbrowser.open(url)
	cf.close()

def init():
	glipper.registerEntry("No-Paste", activated)

def showPreferences():
	preferences().show()


#config file class:
class confFile:
	def __init__(self, mode):
		self.mode = mode

		dir = os.environ["HOME"] + "/.glipper/plugins"
		if (mode == "r") and (not os.path.exists(dir + "/nopaste.conf")):
			self.lang = 13
			self.nick = "glipper user"
			return
		if not os.path.exists(dir):
			os.makedirs(dir)
		self.file = open(dir + "/nopaste.conf", mode)

		if mode == "r":
			self.lang = int(self.file.readline()[:-1])
			self.nick = self.file.readline()[:-1]

	def setLang(self, lang):
		self.lang = lang
	def getLang(self):
		return self.lang
	def setNick(self, nick):
		self.nick = nick
	def getNick(self):
		return self.nick
	def close(self):
		if not 'file' in dir(self):
			return
		try:
			if self.mode == "w":
				self.file.write(str(self.lang) + "\n")
				self.file.write(self.nick + "\n")
		finally:
			self.file.close()

#preferences dialog:

import pygtk
pygtk.require('2.0')
import gtk
import gtk.glade

class preferences:
	def __init__(self):
		gladeFile = gtk.glade.XML(os.path.dirname(__file__) + "/nopaste.glade")
		self.prefWind = gladeFile.get_widget("preferences")
		self.prefWind.connect("destroy", self.destroy)
		self.prefWind.show()
		self.nickEntry = gladeFile.get_widget("nickEntry")
		self.langBox = gladeFile.get_widget("langBox")

		gladeFile.signal_autoconnect(self)

		#read configurations
		f = confFile("r")
		self.nickEntry.set_text(f.getNick())
		self.langBox.set_active(f.getLang())
		f.close()

	def show(self):
		gtk.main()

	#EVENTS:
	def on_applyButton_clicked(self, widget):
		f = confFile("w")
		f.setNick(self.nickEntry.get_text())
		f.setLang(self.langBox.get_active())
		f.close()
		self.prefWind.destroy()	

	def destroy(self, widget):
		gtk.main_quit()
		
