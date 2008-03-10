SUBDIRS = keybinder osutils plugins
	
glipper-applet: glipper-applet.py
	sed -e "s|\@PYTHONDIR\@|$(pythondir)/glipper|" $< > $@

glipperbindir = $(libdir)/glipper
glipperbin_SCRIPTS = glipper-applet

glipperdir = $(pythondir)/glipper
glipper_PYTHON = \
	defs.py \
	Applet.py \
	History.py \
	Clipboards.py \
	Properties.py \
	Plugin.py \
	PluginsManager.py \
	About.py \
	__init__.py \
	Keybinder.py
	
BUILT_SOURCES = \
	glipper-applet
	
CLEANFILES = \
	$(BUILT_SOURCES)
	
DISTCLEANFILES = \
	defs.py \
	$(CLEANFILES)
	
EXTRA_DIST = \
	defs.py.in \
	glipper-applet.py
