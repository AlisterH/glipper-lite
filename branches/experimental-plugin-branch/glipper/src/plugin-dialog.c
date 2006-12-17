#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <stdlib.h>
#include <string.h>
#include "plugin-dialog.h"
#include "plugin.h"
#include "utils/glipper-i18n.h"

#define GLADE_XML_FILE "glipper-plugins.glade"

GtkWidget* pluginWin;
GtkWidget* pluginList;

GtkListStore* pluginStore;

enum {
NAME_COLUMN,
DESCRIPTION_COLUMN,
N_COLUMNS
};

void addPluginToList(char* plugin)
{
	plugin_info* info = malloc(sizeof(plugin_info));
	if (get_plugin_info(plugin, info))
	{
		GtkTreeIter iter;
		gtk_list_store_append(pluginStore, &iter);
		gtk_list_store_set(pluginStore, &iter,
			NAME_COLUMN, info->name,
			DESCRIPTION_COLUMN, info->descr,
			-1);
	}
	else
		g_print("Konnte plugininformationen von plugin %s nicht kriegen.", plugin);
	free(info);
}

void addDirToList(char* dir)
{
	GDir* plugindir = g_dir_open(dir, 0, NULL);
	if (plugindir == NULL)
		return;
	char* plugin;
	while (plugin = g_dir_read_name(plugindir))
		if (strstr(plugin, ".py"))
		{
			char* tmp = malloc(strlen(plugin)-2);
			strncpy(tmp, plugin, strlen(plugin)-3);
			tmp[strlen(plugin)-3] = '\0';
			addPluginToList(tmp);
			free(tmp);
		}
	g_dir_close(plugindir);
	//Maybe watch for errors?
}

void refreshPluginList()
{
	addDirToList(PLUGINDIR);
	char* searchModule = g_build_filename(g_get_home_dir(), ".glipper/plugins", NULL);
	addDirToList(searchModule);
	free(searchModule);
}

void showPluginDialog(gpointer data)
{
	char* glade_file;
	GladeXML* gladeWindow;

	//Load interface from glade file
	glade_file = g_build_filename(GLADEDIR, GLADE_XML_FILE, NULL);

	gladeWindow = glade_xml_new(glade_file, "plugin-dialog", NULL);

	//In case we cannot load glade file
	if (gladeWindow == NULL)
	{
		errorDialog(_("Could not load the preferences interface"), glade_file);
		g_free (glade_file);
		return;
	}

	g_free (glade_file);

	pluginWin = glade_xml_get_widget(gladeWindow, "plugin-dialog");
	pluginList = glade_xml_get_widget(gladeWindow, "pluginList");

	//Set up the plugin list model + widget
	pluginStore = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Name", renderer,
							   "text", NAME_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pluginList), column);
	column = gtk_tree_view_column_new_with_attributes ("Description", renderer,
							   "text", DESCRIPTION_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pluginList), column);
	gtk_tree_view_set_model(GTK_TREE_VIEW (pluginList), GTK_TREE_MODEL (pluginStore));

	refreshPluginList();

	//Show plugin dialog
	gtk_widget_show_all(pluginWin);

	//free the glade data
	g_object_unref(gladeWindow);
}
