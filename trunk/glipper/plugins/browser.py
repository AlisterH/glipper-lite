import glipper, webbrowser

def getInfo():
	info = {"Name": "Browser",
		"Description": "This plugin opens the URL in the clipboard in a webbrowser",
		"Preferences": False}
	return info

def open():
	webbrowser.open(glipper.getItem(0))

def init():
	glipper.registerEntry("open in webbrowser", open)
