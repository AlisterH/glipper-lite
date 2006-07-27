AM_CFLAGS = $(FR_CFLAGS) -I$(top_srcdir) \
	-DGLIPPERLOCALEDIR=\""$(glipperlocaledir)"\" -DPIXMAPDIR=\""$(datadir)/pixmaps"\" \
	-DGLADEDIR=\""$(datadir)/glipper"\" 
AM_LDFLAGS = $(FR_LIBS)

noinst_PROGRAMS=glipper
bin_PROGRAMS=glipper

glipper_SOURCES= \
	main.c  preferences.c preferences.h plugin.c plugin.h\
	utils/eggtrayicon.h  utils/glipper-i18n.h utils/eggtrayicon.c \
	utils/keybinder.c utils/eggaccelerators.c
