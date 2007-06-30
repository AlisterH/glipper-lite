SUBDIRS = keybinder gnomedesktop osutils plugins
	
glipper: glipper.py
	sed -e "s|\@PYTHONDIR\@|$(pythondir)/glipper|" $< > $@

glipperbindir = $(libdir)/glipper
glipperbin_SCRIPTS = glipper

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
	glipper
	
CLEANFILES = \
	$(BUILT_SOURCES)
	
DISTCLEANFILES = \
	defs.py \
	$(CLEANFILES)
	
EXTRA_DIST = \
	defs.py.in \
	glipper.py
