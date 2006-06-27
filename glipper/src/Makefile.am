AM_CFLAGS = $(FR_CFLAGS) -I$(top_srcdir) \
	-DGLIPPERLOCALEDIR=\""$(glipperlocaledir)"\" -DPIXMAPDIR=\""$(datadir)/pixmaps"\" \
	-DGLADEDIR=\""$(datadir)/glipper"\" 
AM_LDFLAGS = $(FR_LIBS)

noinst_PROGRAMS=glipper
bin_PROGRAMS=glipper

glipper_SOURCES= \
	eggtrayicon.c  main.c  preferences.c \
	eggtrayicon.h  glipper-i18n.h  preferences.h \
	utils/keybinder.c utils/eggaccelerators.c
