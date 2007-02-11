AM_CFLAGS = $(FR_CFLAGS) -I$(top_srcdir) \
	-DGNOMELOCALEDIR=\""$(gnomelocaledir)"\" -DPIXMAPDIR=\""$(datadir)/pixmaps"\" \
	-DGLADEDIR=\""$(datadir)/glipper"\" 

libexec_PROGRAMS=glipper

glipper_SOURCES= \
	main.c main.h preferences.c preferences.h \
	utils/eggtrayicon.h  utils/glipper-i18n.h utils/eggtrayicon.c \
	utils/keybinder.c utils/eggaccelerators.c utils/keybinder.h

glipper_LDADD = $(FR_LIBS)
