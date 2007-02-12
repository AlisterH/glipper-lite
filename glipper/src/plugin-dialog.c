/* Glipper - Clipboardmanager for Gnome
 * Copyright (C) 2006 Sven Rech <svenrech@gmx.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
	
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

static GtkWidget* pluginWin;
static GtkWidget* pluginList;
static GtkWidget* startButton;
static GtkWidget* preferencesButton;
static GtkWidget* refreshButton;
static GtkWidget* closeButton;

static GtkListStore* pluginStore;

enum {
	FILE_COLUMN,
	ISRUNNING_COLUMN,
	NAME_COLUMN,
	DESCRIPTION_COLUMN,
	PREFERENCES_COLUMN,
	N_COLUMNS
};

static void setStartButton(int running)
{
		if (running)
		{
			gtk_button_set_label(GTK_BUTTON(startButton), _("stop plugin"));
			gtk_button_set_image(GTK_BUTTON(startButton), 
                            gtk_image_new_from_stock(GTK_STOCK_CANCEL, GTK_ICON_SIZE_SMALL_TOOLBAR));
		}
		else
		{
			gtk_button_set_label(GTK_BUTTON(startButton), _("start plugin"));
			gtk_button_set_image(GTK_BUTTON(startButton), 
                            gtk_image_new_from_stock(GTK_STOCK_APPLY, GTK_ICON_SIZE_SMALL_TOOLBAR));
		}
}

static void addPluginToList(char* plugin)
{
	plugin_info* info = malloc(sizeof(plugin_info));
	if (get_plugin_info(plugin, info))
	{
		GtkTreeIter iter;
		gtk_list_store_append(pluginStore, &iter);
		gtk_list_store_set(pluginStore, &iter,
			FILE_COLUMN, plugin,
			ISRUNNING_COLUMN, info->isrunning,
			NAME_COLUMN, info->name,
			DESCRIPTION_COLUMN, info->descr,
			PREFERENCES_COLUMN, info->preferences,
			-1);
	}
	else
		g_print("couldn't retrieve informations for plugin %s!\n", plugin);
	free(info);
}

static void addDirToList(char* dir)
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

static void refreshPluginList()
{
	gtk_list_store_clear(pluginStore);
	gtk_widget_set_sensitive(preferencesButton, 0);
	gtk_widget_set_sensitive(startButton, 0);
	setStartButton(0);

	addDirToList(PLUGINDIR);
	char* searchModule = g_build_filename(g_get_home_dir(), ".glipper/plugins", NULL);
	addDirToList(searchModule);
	free(searchModule);
}

////////////////////Button signals/////////////////////////////////////////

static void on_startButton_clicked (GtkButton *button, gpointer user_data)
{
        GtkTreeIter iter;
	if (gtk_tree_selection_get_selected (gtk_tree_view_get_selection(GTK_TREE_VIEW(pluginList)), &pluginStore, &iter))
        {
		char* file;
		gtk_tree_model_get(GTK_TREE_MODEL(pluginStore), &iter, FILE_COLUMN, &file, -1);
		int isrunning;
		gtk_tree_model_get(GTK_TREE_MODEL(pluginStore), &iter, ISRUNNING_COLUMN, &isrunning, -1);
		if (!isrunning)
			start_plugin(file);
		else
			stop_plugin(file);
		setStartButton(!isrunning);
		gtk_list_store_set(pluginStore, &iter,
			ISRUNNING_COLUMN, !isrunning,
			-1);
		free(file);
        }
}

static void on_preferencesButton_clicked (GtkButton *button, gpointer user_data)
{
        GtkTreeIter iter;
	if (gtk_tree_selection_get_selected (gtk_tree_view_get_selection(GTK_TREE_VIEW(pluginList)), &pluginStore, &iter))
        {
		char* file;
		gtk_tree_model_get(GTK_TREE_MODEL(pluginStore), &iter, FILE_COLUMN, &file, -1);
		plugin_showPreferences(file);
		free(file);
        }
}

static void on_refreshButton_clicked (GtkButton *button, gpointer user_data)
{
	refreshPluginList();
}

static void on_closeButton_clicked (GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy(pluginWin);
}

static void on_pluginList_selection_changed (GtkTreeSelection *selection, gpointer data)
{
	gtk_widget_set_sensitive(startButton, 1);
        GtkTreeIter iter;
	if (gtk_tree_selection_get_selected (selection, &pluginStore, &iter))
        {
		int isrunning, preferences;
		gtk_tree_model_get(GTK_TREE_MODEL(pluginStore), &iter, ISRUNNING_COLUMN, &isrunning, 
				PREFERENCES_COLUMN, &preferences, -1);
		gtk_widget_set_sensitive(preferencesButton, preferences);
		setStartButton(isrunning);
	}
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
	startButton = glade_xml_get_widget(gladeWindow, "startButton");
	preferencesButton = glade_xml_get_widget(gladeWindow, "preferencesButton");
	refreshButton = glade_xml_get_widget(gladeWindow, "refreshButton");
	closeButton = glade_xml_get_widget(gladeWindow, "closeButton");

	//Connect signals to handlers
	g_signal_connect(startButton, "clicked",
		G_CALLBACK (on_startButton_clicked), NULL);
	g_signal_connect(preferencesButton, "clicked",
		G_CALLBACK (on_preferencesButton_clicked), NULL);
	g_signal_connect(refreshButton, "clicked",
		G_CALLBACK (on_refreshButton_clicked), NULL);
	g_signal_connect(closeButton, "clicked",
		G_CALLBACK (on_closeButton_clicked), NULL);
	g_signal_connect(gtk_tree_view_get_selection(GTK_TREE_VIEW(pluginList)), "changed",
		G_CALLBACK(on_pluginList_selection_changed), NULL);

	//Set up the plugin list model + widget
	pluginStore = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	renderer = gtk_cell_renderer_toggle_new ();
	g_object_set(renderer, "activatable", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes ("Running?", renderer,
							   "active", ISRUNNING_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pluginList), column);
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
