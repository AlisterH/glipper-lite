#!/usr/bin/env python
#
# (C) 2007 Sven Rech.
# Licensed under the GNU GPL.

import gobject
gobject.threads_init()

import gtk, gnomeapplet
import getopt, sys
from os.path import *

# Allow to use uninstalled
def _check(path):
	return exists(path) and isdir(path) and isfile(path+"/AUTHORS")

name = join(dirname(__file__), '..')
if _check(name):
	print 'Running uninstalled glipper, modifying PYTHONPATH'
	sys.path.insert(0, abspath(name))
else:
	sys.path.insert(0, abspath("@PYTHONDIR@"))
	print "Running installed glipper using [@PYTHONDIR@:$PYTHONPATH]"

# Now the path is set, import our applet
import glipper, glipper.Applet, glipper.defs

try:
	# attempt to set a name for killall
	import glipper.osutils
	glipper.osutils.set_process_name ("glipper")
except:
	print "Unable to set processName"

import gettext, locale
gettext.bindtextdomain('glipper', abspath(join(glipper.defs.DATA_DIR, "locale")))
if hasattr(gettext, 'bind_textdomain_codeset'):
	gettext.bind_textdomain_codeset('glipper','UTF-8')
gettext.textdomain('glipper')

locale.bindtextdomain('glipper', abspath(join(glipper.defs.DATA_DIR, "locale")))
if hasattr(locale, 'bind_textdomain_codeset'):
	locale.bind_textdomain_codeset('glipper','UTF-8')
locale.textdomain('glipper')

def applet_factory(applet, iid):
	print 'Starting Glipper instance:', applet, iid
	glipper.Applet.Applet(applet)
	return True

# Return a standalone window that holds the applet
def build_window():
	app = gtk.Window(gtk.WINDOW_TOPLEVEL)
	app.set_title("Glipper")
	app.connect("destroy", gtk.main_quit)
	app.set_property('resizable', False)
	
	applet = gnomeapplet.Applet()
	applet.get_orient = lambda: gnomeapplet.ORIENT_DOWN
	applet_factory(applet, None)
	applet.reparent(app)
		
	app.show_all()
	
	return app
		
def usage():
	print """=== Glipper: Usage
$ glipper [OPTIONS]

OPTIONS:
	-h, --help		Print this help notice.
	-w, --window	Launch the applet in a standalone window for test purposes (default=no).
	"""
	sys.exit()
	
if __name__ == "__main__":	
	standalone = False
	do_trace = False
	
	try:
		opts, args = getopt.getopt(sys.argv[1:], "hw", ["help", "window"])
	except getopt.GetoptError:
		# Unknown args were passed, we fallback to bahave as if
		# no options were passed
		print "WARNING: Unknown arguments passed, using defaults."
		opts = []
		args = sys.argv[1:]
	
	for o, a in opts:
		if o in ("-h", "--help"):
			usage()
		elif o in ("-w", "--window"):
			standalone = True

	print 'Running with options:', {
		'standalone': standalone,
	}
	
	if standalone:
		import gnome
		gnome.init(glipper.defs.PACKAGE, glipper.defs.VERSION)
		build_window()
		
		gtk.main()
		
	else:
		gnomeapplet.bonobo_factory(
			"OAFIID:Glipper_Factory",
			gnomeapplet.Applet.__gtype__,
			glipper.defs.PACKAGE,
			glipper.defs.VERSION,
			applet_factory)

