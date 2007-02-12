icondir = $(datadir)/pixmaps
icon_DATA = glipper.png

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

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GlipperApplet.xml

EXTRA_DIST =            \
	$(icon_DATA)        \
	$(glade_DATA)	\
	$(schemas_DATA) \
	$(ui_DATA) \
	$(server_in_files).in 

DISTCLEANFILES =		\
	$(server_in_files)	\
	$(server_DATA)	

install-data-hook:
	gconf-schemas --register $(datadir)/gconf/schemas/glipper.schemas
