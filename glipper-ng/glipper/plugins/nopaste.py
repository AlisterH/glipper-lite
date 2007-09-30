/* Glipper - Clipboardmanager for GNOME
 * Copyright (C) 2007 Glipper Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

import httplib, urllib, os, os.path, webbrowser, threading
import glipper

from gettext import gettext as _

def info():
   info = {"Name": _("Nopaste"), 
      "Description": _("Paste the entry of your clipboard to a Nopaste service"),
      "Preferences": True}
   return info

class RafbNet(threading.Thread):
   def __init__(self, lang, nick, desc, text):
      threading.Thread.__init__(self)
      self.lang = lang
      self.nick = desc
      self.desc = desc	      
      self.text = text

   def run(self):
      conn = httplib.HTTPConnection("rafb.net")
      params = urllib.urlencode({"lang": self.lang, "nick": self.nick, 
         "desc": self.desc, "text": self.text, "tabs": "no"})
      headers = {"Content-type": "application/x-www-form-urlencoded", "Accept": "text/plain"}
      conn.request("POST", "/paste/paste.php", params, headers)
      url = conn.getresponse().getheader("location")
      conn.close()
      webbrowser.open(url)

def activated(menu):
   languageList = ("C89", "C", "C++", "C#", "Java", "Pascal", "Perl", "PHP", 
         "PL/I", "Python", "Ruby", "SQL", "VB", "Plain Text")
   cf = confFile("r")
   rafbnet = RafbNet(languageList[cf.getLang()], cf.getNick(), _("pasted by Glipper"), glipper.get_history_item(0))
   cf.close()
   rafbnet.start()

def init():
   item = gtk.MenuItem(_("Nopaste"))
   item.connect('activate', activated)
   glipper.add_menu_item(item)

def on_show_preferences(parent):
   preferences(parent).show()


#config file class:
class confFile:
   def __init__(self, mode):
      self.mode = mode

      dir = os.environ["HOME"] + "/.glipper/plugins"
      if (mode == "r") and (not os.path.exists(dir + "/nopaste.conf")):
         self.lang = 13
         self.nick = _("Glipper user")
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
import gtk
import gtk.glade

class preferences:
   def __init__(self, parent):
      gladeFile = gtk.glade.XML(os.path.dirname(__file__) + "/nopaste.glade")
      self.prefWind = gladeFile.get_widget("preferences")
      self.prefWind.set_transient_for(parent)
      self.nickEntry = gladeFile.get_widget("nickEntry")
      self.langBox = gladeFile.get_widget("langBox")
      self.prefWind.connect('response', self.on_prefWind_response)

      #read configurations
      f = confFile("r")
      self.nickEntry.set_text(f.getNick())
      self.langBox.set_active(f.getLang())
      f.close()

   def destroy(self, window):
      window.destroy()

   def show(self):
      self.prefWind.show_all()

   #EVENTS:
   def on_prefWind_response(self, widget, response):
      if response == gtk.RESPONSE_DELETE_EVENT or response == gtk.RESPONSE_CLOSE:
         f = confFile("w")
         f.setNick(self.nickEntry.get_text())
         f.setLang(self.langBox.get_active())
         f.close()
         widget.destroy()   
      
