icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = scalable/glipper.png
22icondir = $(datadir)/icons/hicolor/22x22/apps
22icon_DATA = 22x22/glipper.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps
svgicon_DATA = scalable/glipper.svg
gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

uninstall-hook: update-icon-cache

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

gladedir = $(datadir)/glipper
glade_DATA = glipper-properties.glade glipper-plugins.glade

schemasdir = $(datadir)/gconf/schemas
schemas_DATA = glipper.schemas

serverdir       = $(libdir)/bonobo/servers
server_in_files = GlipperApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)
$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@
@INTLTOOL_SERVER_RULE@
@INTLTOOL_SCHEMAS_RULE@
uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GlipperApplet.xml

EXTRA_DIST = \
   $(icon_DATA) \
   $(22icon_DATA) \
   $(svgicon_DATA) \
	$(glade_DATA)	\
	$(schemas_DATA) \
	$(ui_DATA) \
	$(server_in_files).in 

DISTCLEANFILES =		\
	$(server_in_files)	\
	$(server_DATA)	

install-data-hook:
   update-icon-cache \
   GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA)
