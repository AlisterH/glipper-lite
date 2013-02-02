desktopdir = $(datadir)/applications
desktop_in_files = glipper.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

icondir = $(datadir)/pixmaps
icon_DATA = glipper.png

uidir = $(datadir)/glipper
ui_DATA = glipper-properties.ui

EXTRA_DIST =            \
	$(desktop_in_files) \
	$(icon_DATA)        \
	$(ui_DATA)	\
	#$(desktop_DATA)

DISTCLEANFILES =		\
	$(desktop_DATA)				
