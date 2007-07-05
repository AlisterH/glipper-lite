from gettext import gettext as _
import gtk, gtk.gdk, gnomevfs, gobject
import glipper


def on_email(about, mail):
   gnomevfs.url_show("mailto:%s" % mail)

def on_url(about, link):
   gnomevfs.url_show(link)

gtk.about_dialog_set_email_hook(on_email)
gtk.about_dialog_set_url_hook(on_url)

class About(object):
   __instance = None
   
   def __init__(self, parent):
      if About.__instance == None:
         About.__instance = self
      else:
         About.__instance.about.present()
         return
         
      self.about = gtk.AboutDialog()
   
      infos = {
         "name" : _("Glipper"),
         "logo-icon-name" : "glipper",
         "version" : glipper.VERSION,
         "comments" : _("A clipboard manager."),
         "copyright" : "Copyright © 2007 Sven Rech, Eugenio Depalo, Karderio.",
         "website" : "http://glipper.sourceforge.net",
         "website-label" : _("Glipper website"),
      }

      self.about.set_authors(["Sven Rech <sven@gmx.de>", "Eugenio Depalo <eugeniodepalo@mac.com>", "Karderio <karderio@gmail.com>"])
      #   about.set_artists([])
      #   about.set_documenters([])
   
      #translators: These appear in the About dialog, usual format applies.
      self.about.set_translator_credits( _("translator-credits") )
   
      for prop, val in infos.items():
         self.about.set_property(prop, val)
   
      self.about.connect("response", self.destroy)
      self.about.set_screen(parent.get_screen())
      self.about.show_all()
      
   def destroy(self, dialog, response):
      dialog.destroy()
      About.__instance = None
