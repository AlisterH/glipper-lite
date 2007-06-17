AM_CFLAGS = $(FR_CFLAGS) -I$(top_srcdir) -I/usr/include/python2.5/\
	-DGNOMELOCALEDIR=\""$(gnomelocaledir)"\" -DPIXMAPDIR=\""$(datadir)/pixmaps"\" \
	-DGLADEDIR=\""$(datadir)/glipper"\" -DPLUGINDIR=\""$(datadir)/glipper/plugins"\"
AM_LDFLAGS = $(FR_LIBS) -lpython2.5

libexec_PROGRAMS=glipper

glipper_SOURCES= \
	main.c main.h preferences.c preferences.h \
	utils/glipper-i18n.h utils/keybinder.c \
	utils/eggaccelerators.c utils/keybinder.h \
	plugin-dialog.c plugin-dialog.h \
	plugin.c plugin.h

glipper_LDADD = $(FR_LIBS)
