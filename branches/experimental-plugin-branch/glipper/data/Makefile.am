desktopdir = $(datadir)/applications
desktop_in_files = glipper.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

icondir = $(datadir)/pixmaps
icon_DATA = glipper.png

gladedir = $(datadir)/glipper
glade_DATA = glipper-properties.glade

EXTRA_DIST =            \
	$(desktop_in_files) \
	$(icon_DATA)        \
	$(glade_DATA)	\
	#$(desktop_DATA)

DISTCLEANFILES =		\
	$(desktop_DATA)				
