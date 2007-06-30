import glipper, webbrowser

def getInfo():
	info = {"Name": "Browser",
		"Description": "This plugin opens the URL in the clipboard in a webbrowser",
		"Preferences": False}
	return info

def open():
	url = glipper.getItem(0)
	try:
		webbrowser.open(url)
	except:
		print "couldn't open \"" + url + "\""

def init():
	glipper.registerEntry("open in webbrowser", open)
