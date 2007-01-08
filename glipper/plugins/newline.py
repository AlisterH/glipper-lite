import glipper

def newItem(arg):
	glipper.setItem(0, glipper.getItem(0)+'\n')
	glipper.setActiveItem(0)

def getInfo():
	info = {"Name": "New Line",
		"Description": "This plugin adds a newline character at the end of items in the clipboard",
		"Preferences": False}
	return info
