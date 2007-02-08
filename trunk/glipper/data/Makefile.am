desktopdir = $(datadir)/applications
desktop_in_files = glipper.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

icondir = $(datadir)/pixmaps
icon_DATA = glipper.png

gladedir = $(datadir)/glipper
glade_DATA = glipper-properties.glade

schemasdir = $(datadir)/gconf/schemas
schemas_DATA = glipper.schemas

serverdir       = $(libdir)/bonobo/servers
server_in_files = GlipperApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@prefix\@|$(prefix)|" $< > $@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GlipperApplet.xml

@INTLTOOL_SERVER_RULE@

EXTRA_DIST =            \
	$(desktop_in_files) \
	$(icon_DATA)        \
	$(glade_DATA)	\
	$(schemas_DATA) \
	$(ui_DATA) \
	GlipperApplet_Factory.server.in.in

DISTCLEANFILES =		\
	$(desktop_DATA)		\
	$(server_in_files)	\
	$(server_DATA)	

install-data-hook:
	gconf-schemas --register $(datadir)/gconf/schemas/glipper.schemas
