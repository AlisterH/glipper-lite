from gettext import gettext as _
import gtk, gtk.gdk, gnomevfs, gobject
import glipper


def on_email(about, mail):
	gnomevfs.url_show("mailto:%s" % mail)

def on_url(about, link):
	gnomevfs.url_show(link)

gtk.about_dialog_set_email_hook(on_email)
gtk.about_dialog_set_url_hook(on_url)

def show_about(parent):
	about = gtk.AboutDialog()
	
	infos = {
		"name" : _("Glipper"),
		"logo-icon-name" : "glipper",
		"version" : glipper.VERSION,
		"comments" : _("A clipboard manager."),
		"copyright" : "Copyright © 2007 Sven Rech, Eugenio Depalo, Karderio.",
		"website" : "http://glipper.sourceforge.net",
		"website-label" : _("Glipper website"),
	}

	about.set_authors(["Sven Rech <sven@gmx.de>", "Eugenio Depalo <eugeniodepalo@mac.com>", "Karderio <karderio@gmail.com>"])
#	about.set_artists([])
#	about.set_documenters([])
	
	#translators: These appear in the About dialog, usual format applies.
	about.set_translator_credits( _("translator-credits") )
	
	for prop, val in infos.items():
		about.set_property(prop, val)
	
	about.connect("response", lambda self, *args: self.destroy())
	about.set_screen(parent.get_screen())
	about.show_all()
