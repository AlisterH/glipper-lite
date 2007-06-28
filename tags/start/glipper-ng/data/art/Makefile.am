# ******************************************************************************
# Images and icon
# ******************************************************************************

22icondir = $(datadir)/icons/hicolor/22x22/apps
22icon_DATA = 22x22/glipper.png

icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = scalable/glipper.png

svgicondir = $(datadir)/icons/hicolor/scalable/apps
svgicon_DATA = scalable/glipper.svg

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

EXTRA_DIST = \
	$(22icon_DATA) \
	$(icon_DATA) \
	$(svgicon_DATA)
