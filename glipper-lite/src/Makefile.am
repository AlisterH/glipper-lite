AM_CFLAGS = $(FR_CFLAGS) -I$(top_srcdir) \
	-DGLIPPERLOCALEDIR=\""$(glipperlocaledir)"\" -DPIXMAPDIR=\""$(datadir)/pixmaps"\" \
	-DUI_DIR=\""$(datadir)/glipper-lite"\" 

noinst_PROGRAMS=glipper-lite
bin_PROGRAMS=glipper-lite

glipper_lite_SOURCES= \
	main.c main.h preferences.c preferences.h \
	utils/glipper-i18n.h \
	utils/keybinder.c utils/eggaccelerators.c utils/keybinder.h utils/eggaccelerators.h

extra_glipper_lite_SOURCES= \
	utils/eggtrayicon.h utils/eggtrayicon.c
	
glipper_lite_LDADD = $(FR_LIBS)
