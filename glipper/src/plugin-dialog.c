/* Glipper - Clipboardmanager for GNOME
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

static GConfClient *global_conf;

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
   AUTOSTART_COLUMN,
	NAME_COLUMN,
	DESCRIPTION_COLUMN,
	PREFERENCES_COLUMN,
	N_COLUMNS
};

static void addPluginToList(char* plugin)
{
	plugin_info* info = malloc(sizeof(plugin_info));
	if (get_plugin_info(plugin, info))
	{
		GtkTreeIter iter;
      gboolean autostart = FALSE;
      GSList *list = gconf_client_get_list(global_conf, AUTOSTART_PLUGINS_KEY, GCONF_VALUE_STRING, NULL);

      for(; list && !autostart; list = g_slist_next(list))
         if(!g_ascii_strcasecmp(list->data, (gchar*)plugin))
            autostart = TRUE;

		gtk_list_store_append(pluginStore, &iter);
		gtk_list_store_set(pluginStore, &iter,
			FILE_COLUMN, plugin,
			ISRUNNING_COLUMN, info->isrunning,
         AUTOSTART_COLUMN, autostart,
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

	addDirToList(PLUGINDIR);
	char* searchModule = g_build_filename(g_get_home_dir(), ".glipper/plugins", NULL);
	addDirToList(searchModule);
	free(searchModule);
}

////////////////////Button signals/////////////////////////////////////////

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
        GtkTreeIter iter;
	if (gtk_tree_selection_get_selected (selection, &pluginStore, &iter))
        {
		int isrunning, preferences;
		gtk_tree_model_get(GTK_TREE_MODEL(pluginStore), &iter, ISRUNNING_COLUMN, &isrunning, 
				PREFERENCES_COLUMN, &preferences, -1);
		gtk_widget_set_sensitive(preferencesButton, preferences);
	}
}

static void on_enabledToggled(GtkCellRenderer* toggle, gchar *path, gpointer data) {
   GtkTreeIter iter;
   GtkTreeModel *model = GTK_TREE_MODEL(pluginStore);
   GtkTreePath *toggle_path = gtk_tree_path_new_from_string(path);

   if(gtk_tree_model_get_iter(model, &iter, toggle_path)) {
      gboolean enabled;
      gchar* file;
      gtk_tree_model_get(model, &iter, ISRUNNING_COLUMN, &enabled, FILE_COLUMN, &file, -1);
      
      if(enabled)
         stop_plugin(file);
      else
         start_plugin(file);

      gtk_list_store_set(pluginStore, &iter, ISRUNNING_COLUMN, !enabled, -1);

      g_free(file);
   }

   g_free(toggle_path);  
}

static void on_autostartToggled(GtkCellRenderer* toggle, gchar *path, gpointer data) {
   GtkTreeIter iter;
   GtkTreeModel *model = GTK_TREE_MODEL(pluginStore);
   GtkTreePath *toggle_path = gtk_tree_path_new_from_string(path);

   if(gtk_tree_model_get_iter(model, &iter, toggle_path)) {
      gboolean autostart;
      gchar* file;
      GSList* list = gconf_client_get_list(global_conf, AUTOSTART_PLUGINS_KEY, GCONF_VALUE_STRING, NULL);
      GSList* temp;

      gtk_tree_model_get(model, &iter, AUTOSTART_COLUMN, &autostart, FILE_COLUMN, &file, -1);
      
      if(autostart) {
       for(temp = list; temp; temp = g_slist_next(temp))
         if(!g_ascii_strcasecmp((gchar*)temp->data, file))
			   list = g_slist_remove(list, temp->data);
      }
      else
         list = g_slist_append(list, file);

      gconf_client_set_list(global_conf, AUTOSTART_PLUGINS_KEY, GCONF_VALUE_STRING, list, NULL);

      gtk_list_store_set(pluginStore, &iter, AUTOSTART_COLUMN, !autostart, -1);

      g_free(file);
   }

   g_free(toggle_path);  
}

static void
autostart_plugins_key_changed_callback(GConfClient *client,
		    guint        cnxn_id,
		    GConfEntry  *entry,
		    gpointer     user_data)
{
   GtkTreeModel *model = GTK_TREE_MODEL(pluginStore);
   GtkTreeIter iter;
   gboolean valid;
   GSList* list = gconf_client_get_list(global_conf, AUTOSTART_PLUGINS_KEY, GCONF_VALUE_STRING, NULL);
   GSList* temp;

   valid = gtk_tree_model_get_iter_first(model, &iter);

   while (valid)
   {
      gchar *file;
      gboolean autostart = FALSE;
      gtk_tree_model_get(model, &iter, FILE_COLUMN, &file, -1);

      for(temp = list; temp && !autostart; temp = g_slist_next(temp))
         if(!g_ascii_strcasecmp((gchar*)temp->data, file))
            autostart = TRUE;

      gtk_list_store_set(pluginStore, &iter, AUTOSTART_COLUMN, autostart, -1);
      valid = gtk_tree_model_iter_next(model, &iter);

      g_free(file);
   }
}

void initPluginDialog(GConfClient *conf)
{
	g_return_if_fail (conf != NULL);
	g_return_if_fail (global_conf == NULL);

	global_conf = conf;
	g_object_ref (G_OBJECT (global_conf));

	gconf_client_notify_add (conf,
                           AUTOSTART_PLUGINS_KEY,
                           autostart_plugins_key_changed_callback,
                           NULL,
                           NULL, NULL);
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
		errorDialog(_("Could not load the plugin interface"), glade_file);
		g_free (glade_file);
		return;
	}

	g_free (glade_file);

	pluginWin = glade_xml_get_widget(gladeWindow, "plugin-dialog");
	pluginList = glade_xml_get_widget(gladeWindow, "pluginList");
	preferencesButton = glade_xml_get_widget(gladeWindow, "preferencesButton");
	refreshButton = glade_xml_get_widget(gladeWindow, "refreshButton");
	closeButton = glade_xml_get_widget(gladeWindow, "closeButton");

	//Connect signals to handlers
	g_signal_connect(preferencesButton, "clicked",
		G_CALLBACK (on_preferencesButton_clicked), NULL);
	g_signal_connect(refreshButton, "clicked",
		G_CALLBACK (on_refreshButton_clicked), NULL);
	g_signal_connect(closeButton, "clicked",
		G_CALLBACK (on_closeButton_clicked), NULL);
	g_signal_connect(gtk_tree_view_get_selection(GTK_TREE_VIEW(pluginList)), "changed",
		G_CALLBACK(on_pluginList_selection_changed), NULL);

	//Set up the plugin list model + widget
	pluginStore = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	renderer = gtk_cell_renderer_toggle_new ();
   g_signal_connect(renderer, "toggled", on_enabledToggled, NULL);
	column = gtk_tree_view_column_new_with_attributes ("Enabled", renderer,
							   "active", ISRUNNING_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (pluginList), column);
	renderer = gtk_cell_renderer_toggle_new ();
   g_signal_connect(renderer, "toggled", on_autostartToggled, NULL);
	column = gtk_tree_view_column_new_with_attributes ("Autostart", renderer,
							   "active", AUTOSTART_COLUMN,
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
