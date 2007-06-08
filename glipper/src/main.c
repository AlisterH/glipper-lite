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
				 
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "preferences.h"
#include "plugin-dialog.h"
#include "utils/glipper-i18n.h"
#include "utils/keybinder.h"
#include "plugin.h"
#include "main.h"
#include <libgnome/gnome-help.h>
#include <panel-applet.h>

static GConfClient* conf = NULL;

typedef struct {
    GtkClipboard* board; //The clipboard object itself
    gchar* data;	 //A pointer to the data on the history list
    gboolean afterClear; //TRUE when "data" is allocated after a clear
    gboolean isBin;	 //TRUE when clipboard set to binary data w/o text
} ClipStruct;

ClipStruct PrimaryClip;
ClipStruct DefaultClip;
GSList* history = NULL;
GtkWidget* historyMenu = NULL;
GtkWidget* popupMenu;
GtkTooltips* toolTip; //The applet's tooltip
GtkWidget* image;
gint mainTimeout;
gchar* popupKey;
gint appletSize = 22;
int hasChanged = 1;
GStaticRecMutex mutex; //a global mutex for making the used data structures thread safe

void errorDialog(gchar* error_msg, gchar* secondaryText);
void saveHistory();

void getClipboards()
{
	/*There exists two different Clipboards:
	  PrimaryClipboard: Marked with mouse
	  DefaultClipboard: copied with Strg+C
	*/
	DefaultClip.board = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	PrimaryClip.board = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
}

void deleteHistory(GtkMenuItem* menuItem, gpointer user_data)
{
	if (!history)
		return;
	hasChanged = 1;
	if (PrimaryClip.data && !PrimaryClip.afterClear)
	{
		PrimaryClip.data = g_strdup(PrimaryClip.data);
		PrimaryClip.afterClear = TRUE;
	}
	if (DefaultClip.data && !DefaultClip.afterClear)
	{
		DefaultClip.data = g_strdup(DefaultClip.data);
		DefaultClip.afterClear = TRUE;
	}
	g_slist_free(history);
	history = NULL;
	if (gconf_client_get_bool(conf, SAVE_HISTORY_KEY, NULL))
		saveHistory();
	plugins_historyChanged();
}

//add history entry to menu:
GtkWidget* addHistMenuItem(gchar* item)
{
	static GtkTooltips* toolTip; //The tooltip for bold item

	if (toolTip == NULL) toolTip = gtk_tooltips_new();

	//we have to cut the string to "maxItemLength" characters:
	GString* ellipseData = g_string_new(item ? item : "(Binary Data)");
	gint maxItemLength = gconf_client_get_int(conf, MAX_ITEM_LENGTH_KEY, NULL);
	if (ellipseData->len > maxItemLength)
	{
		ellipseData = g_string_erase(ellipseData, maxItemLength/2, 
									ellipseData->len-maxItemLength);
		ellipseData = g_string_insert(ellipseData, ellipseData->len/2, "...");
	}
	//remove special escape character (\n and \t):
	{ 
		int x;
		for (x=0; x<=ellipseData->len; x++)
		{
			if ((ellipseData->str[x]=='\n')||(ellipseData->str[x]=='\t'))
				ellipseData->str[x] = ' ';
		}
	}
	//The cutted string is now stored in "ellipseData"
	GtkWidget* MenuItem = gtk_menu_item_new_with_label(ellipseData->str);
	//if this string belongs to the default Clipboard, we tag it (presumed markDefault is true):
	int markDefault = gconf_client_get_bool(conf, MARK_DEFAULT_ENTRY_KEY, NULL);
	int useDefault = gconf_client_get_bool(conf, USE_DEFAULT_CLIPBOARD_KEY, NULL);
	int usePrimary = gconf_client_get_bool(conf, USE_PRIMARY_CLIPBOARD_KEY, NULL);
 	if (markDefault && useDefault && usePrimary
 	 && ((item==NULL && DefaultClip.isBin)
 	  || (item!=NULL && !DefaultClip.isBin && DefaultClip.data == item)))
	{
 		GtkLabel* label = (GtkLabel*)gtk_bin_get_child((GtkBin*)MenuItem); 
 		gchar* fmt = item ? "<b>%s</b>" : "<b><i>%s</i></b>";
 		gchar* temp = g_strdup_printf(fmt, ellipseData->str);
 		gtk_label_set_markup(label, temp);
 		g_free(temp);
 
 		//Add description tooltip to bold entry
 		gtk_tooltips_set_tip(toolTip, MenuItem, _("This entry was copied with ctrl+c.\nIt can be pasted with ctrl+v."), "Glipper");
	}
	g_string_free(ellipseData, TRUE);
	gtk_menu_append(historyMenu, MenuItem);
	g_signal_connect(G_OBJECT(MenuItem), "activate", 
							 G_CALLBACK(historyEntryActivate), 
							 item);
	return MenuItem;
}

void createHistMenu()
{
	static GtkTooltips* toolTip = NULL; //The tooltip for first item
	static GtkWidget* menuHeader = NULL; //A nice header for the menu 

	if (toolTip == NULL) 
		toolTip = gtk_tooltips_new();

	//Create menu header
	//TODO : replace the menu item with non interactive widgets
	if (menuHeader == NULL)
	{
		GdkPixbuf* pixbuf;
		GError* pix_error = NULL;

		menuHeader = gtk_image_menu_item_new_with_label("");
		pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default(), "glipper", 22, 0, &pix_error);

		//In case we cannot load icon, display error message and exit
		if (pix_error != NULL)
		{
			errorDialog(pix_error->message, 
				_("Cannot load icon. Is the software properly installed ?"));
			exit(1);
		}

		gtk_image_menu_item_set_image((GtkImageMenuItem*)menuHeader, 
			gtk_image_new_from_pixbuf(pixbuf));
	}	
	gtk_label_set_text((GtkLabel*)gtk_bin_get_child((GtkBin*)menuHeader), 
			g_strdup_printf(_("Glipper - Clipboardmanager  (%s)"), gconf_client_get_string(conf, KEY_COMBINATION_KEY, NULL)));

	if (historyMenu != NULL)
	{
		//We remove menuHeader from the old Menu. Before doing so,
		//we have to add a reference, for that it will not be deleted:
		gtk_widget_ref(menuHeader);  
		gtk_container_remove((GtkContainer*)historyMenu, menuHeader); 
		gtk_widget_destroy(historyMenu);
	}	
	historyMenu = gtk_menu_new();

	//Append menu header
	gtk_menu_append(historyMenu, menuHeader);
	gtk_menu_append(historyMenu, gtk_separator_menu_item_new());

	//Now add all history entrys:
	if (history == NULL)
		gtk_menu_append(historyMenu, 
			gtk_menu_item_new_with_label(_("<No Content>")));
	else
	{
		GSList* temp = history;

		//We add the first item manually
		GtkWidget* firstItem;
		if (PrimaryClip.isBin)
			firstItem = addHistMenuItem(NULL);
		else
		{
			firstItem = addHistMenuItem(temp->data);
			temp = g_slist_next(temp);
		}

		//Add description tooltip to first item
		gtk_tooltips_set_tip(toolTip, firstItem, _("This is the last element to be copied.\nIt can be pasted with the middle mouse button."), "Glipper");

		if (temp != NULL || (DefaultClip.isBin && !PrimaryClip.isBin))
		{
			gtk_menu_append((GtkMenu*)historyMenu, gtk_separator_menu_item_new());
			if (DefaultClip.isBin && !PrimaryClip.isBin)
				addHistMenuItem(NULL);
			while (temp != NULL) {
				addHistMenuItem(temp->data);
				temp = g_slist_next(temp);
			}
		}
	}

	gtk_menu_append((GtkMenu*)historyMenu, gtk_separator_menu_item_new());
	//Add all plugin entries:
	menuEntry* it;
	for (it = menuEntryList.next; it != NULL; it = it->next)
	{
		GtkWidget* widget = gtk_image_menu_item_new_with_label(it->label);
		g_signal_connect(G_OBJECT(widget), "activate", G_CALLBACK(plugin_menu_callback), it->callback);
		gtk_menu_append((GtkMenu*)historyMenu, widget);
	}	
	//Add the "clear" button at the bottom of the menu
	GtkWidget* deleteAll = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLEAR, NULL);
	g_signal_connect(G_OBJECT(deleteAll), "activate", G_CALLBACK(deleteHistory), NULL);
	gtk_menu_append((GtkMenu*)historyMenu, deleteAll);

	gtk_widget_show_all(historyMenu);
}

//Delete the bottom element in the menu, or the 2nd-to-bottom if the
//DefaultClipboard item is at the bottom.
void deleteOldElement(int limit)
  {
	if (limit < 2)
		limit = 2;

	GSList* grandparentElement = g_slist_nth(history, limit-2);
	if (grandparentElement == NULL)
		return;

	GSList* parentElement = grandparentElement->next;
	if (parentElement == NULL || parentElement->next == NULL)
		return;

	GSList* cur;
	GSList* prior = parentElement;
	for (cur = prior->next; cur != NULL; prior = cur, cur = prior->next) {
		if (cur->data == DefaultClip.data)
		{
			//Unlink DefaultClip's node from its current spot
			prior->next = cur->next;
			//Move it to the last spot in the retained list
			grandparentElement->next = cur;
			cur->next = parentElement;
			parentElement = cur;
			break;
		}
	}

	g_slist_free(parentElement->next);
	parentElement->next = NULL;
	plugins_historyChanged();
}

void insertInHistory(ClipStruct *clip)
{
	gchar* content = clip->data;

	if (history == NULL || strcmp(content, history->data)!=0)
  	{
  		//g_print(content);
		history = g_slist_prepend(history, content);
  		GSList* temp = history;
		//If the primary selection is growing, replace the previous
		//substring (as long as the string is not needed by the
		//default clipboard).  If the newly-grown string duplicates
		//a later entry, we'll also remove that below.
		if (clip==&PrimaryClip && temp->next!=NULL && temp->next->data!=NULL
		 && DefaultClip.data != temp->next->data) {
			int old_len = strlen(temp->next->data);
			int new_len = strlen(content);
			if (new_len > old_len
			 && (strncmp(temp->next->data, content, old_len) == 0
			  || strcmp(temp->next->data, content+new_len-old_len) == 0))
			{
				GSList* dummy = temp->next;
				if (DefaultClip.data == dummy->data)
					DefaultClip.data = content;
				temp->next = temp->next->next;
				g_slist_free_1(dummy);
			}
		}
		//We look whether the same entry still exists in the history and delete it:
  		while ((temp->next!=NULL)&&(temp->next->data!=NULL)&&
		  (strcmp(temp->next->data, content)!=0))
  		  	temp = temp->next;
  		//If the same entry was found:
  		if (temp->next!=NULL)
  		{
  			GSList* dummy = temp->next;
			if (DefaultClip.data == dummy->data)
				DefaultClip.data = content;
  			temp->next = temp->next->next;
  			g_slist_free_1(dummy);
  		}
  		//We shorten the history if it gets longer than "maxElements":
		deleteOldElement(gconf_client_get_int(conf, MAX_ELEMENTS_KEY, NULL));
  		if (gconf_client_get_bool(conf, SAVE_HISTORY_KEY, NULL))
  			saveHistory();
		plugins_historyChanged();
		plugins_newItem();
  	}
	else
	{
		g_free(content);
		clip->data = history->data;
	}
  }
  
void processContent(ClipStruct *clip)
  {
	gchar* newContent = gtk_clipboard_wait_for_text(clip->board);
  	if (newContent == NULL)
  	{
		gint count;
		GdkAtom *targets;
		int ok = gtk_clipboard_wait_for_targets(clip->board, &targets, &count);
		g_free(targets);
		if (ok)
		{
			hasChanged |= !clip->isBin;
			clip->isBin = TRUE;
		}
		else if (clip->data != NULL)
		{
			gtk_clipboard_set_text(clip->board, clip->data, -1);
			hasChanged = 1;
			clip->isBin = FALSE;
  		}
	}
	else if (clip->data==NULL || strcmp(newContent, clip->data)!=0
	 || (clip == &PrimaryClip && history != NULL && strcmp(newContent, history->data)!=0))
	{
		if (clip->afterClear)
		{
			clip->afterClear = FALSE;
			g_free(clip->data);
		}
		clip->data = newContent;
		insertInHistory(clip);
		hasChanged = 1;
		clip->isBin = FALSE;
	}
	else
	{
		g_free(newContent);
		hasChanged |= !clip->isBin;
		clip->isBin = FALSE;
  	}
  }
  
  gboolean checkClipboard(gpointer data)
  {
	g_static_rec_mutex_lock(&mutex);
  	g_source_remove(mainTimeout);
  	if (gconf_client_get_bool(conf, USE_PRIMARY_CLIPBOARD_KEY, NULL))
		processContent(&DefaultClip);
	if (gconf_client_get_bool(conf, USE_DEFAULT_CLIPBOARD_KEY, NULL))
		processContent(&PrimaryClip);
  	mainTimeout = g_timeout_add(500, checkClipboard, NULL);
	g_static_rec_mutex_unlock(&mutex);
  	return 1;
  }

void historyMenuDeactivate(GtkMenu *menu, gpointer user_data)
{
   GtkWidget *image = GTK_WIDGET(user_data);
   gtk_widget_set_state (image, GTK_STATE_NORMAL);
}

void historyMenuPosition(GtkMenu *menu, gint *_x, gint *_y, gboolean *push_in, gpointer user_data)
{
    GtkWidget *widget = GTK_WIDGET (user_data);
    gint x, y;

    gdk_window_get_origin (widget->window, &x, &y);

    y += widget->allocation.height;
    *_x = x;
    *_y = y;
}

gboolean AppletIconClicked(GtkWidget* widget, GdkEventButton *event, gpointer user_data)
{
	if (event->button != 1)
		return FALSE;

    if (hasChanged) {
        createHistMenu();
    }

    // This is needed to set back the normal state to the applet when the history menu gets deactivated
    g_signal_connect(G_OBJECT(historyMenu), "deactivate", G_CALLBACK(historyMenuDeactivate), widget);

    hasChanged = 0;

	gtk_widget_set_state (GTK_WIDGET (widget), GTK_STATE_SELECTED);

    gtk_menu_popup ((GtkMenu*)historyMenu, NULL, NULL, historyMenuPosition, widget,
				    1, gtk_get_current_event_time());
    return TRUE;
}

void historyEntryActivate(GtkMenuItem* menuItem, gpointer user_data)
{
	g_static_rec_mutex_lock(&mutex);
	if (gconf_client_get_bool(conf, USE_PRIMARY_CLIPBOARD_KEY, NULL))
		gtk_clipboard_set_text(PrimaryClip.board, (gchar*)user_data, -1);
	if (gconf_client_get_bool(conf, USE_DEFAULT_CLIPBOARD_KEY, NULL))
		gtk_clipboard_set_text(DefaultClip.board, (gchar*)user_data, -1);
	g_static_rec_mutex_unlock(&mutex);
	checkClipboard(NULL);
	hasChanged = 1;
}

//Displays help in Yelp
void help(gchar* section)
{
	GError *error;

	error = NULL;
	//gnome_help_display ("glipper", NULL, &error);
        gnome_help_display_desktop(NULL, "glipper", "glipper", 
                                   section, &error);

	if (error)
	{
		errorDialog(_("Could not display help for Glipper"), "Is the help properly installed ?");
		g_error_free (error);
	}
}

void showHelp(BonoboUIComponent *uic, PanelApplet *glipper_applet, const gchar *verbname)
{
	help(NULL);
}

void show_about(BonoboUIComponent *uic, PanelApplet *glipper_applet, const gchar *verbname)
{
	gchar* authors[] = {"Sven Rech <svenrech@gmx.de>", "Karderio <karderio at gmail dot com>", "Eugenio Depalo <eugeniodepalo@mac.com>", NULL};

	gchar* license = 
"This library is free software; you can redistribute it and/or\n"
"modify it under the terms of the GNU Lesser General Public\n"
"License as published by the Free Software Foundation; either\n"
"version 2.1 of the License, or (at your option) any later version.\n"
"\n"
"This library is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
"Lesser General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU Lesser General Public\n"
"License along with this library; if not, write to the Free Software\n"
"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA";

	gtk_show_about_dialog(NULL, 
		"authors", authors,
		"copyright", "Copyright © 2006 Sven Rech",
		"translator-credits", _("translator-credits"),
		"license", license,
		"name", "Glipper",
		"comments", _("Clipboardmanager for Gnome"),
		"logo-icon-name", "glipper",
		"website", "http://glipper.sourceforge.net/",
		"version", VERSION,
		 NULL);
}

void keyhandler(char *keystring, gpointer user_data)
{
	if (hasChanged)
		createHistMenu();
	gtk_menu_popup ((GtkMenu*)historyMenu, NULL, NULL, NULL, NULL,
					0, gtk_get_current_event_time());
}

//trys to open (or create) a file in "~/.glipper" for writing purposes:
FILE* writeGlipperFile(char* filename)
{
	gchar* directory = g_build_path("/", g_get_home_dir(), ".glipper", NULL);
	gchar* path = g_build_filename(directory, filename, NULL);
	FILE* file = fopen(path, "w");
	if (file == NULL)
	{
		if (mkdir(directory, S_IRWXU)==0)  //Trys to create the folder if open failes
		{
			file = fopen(path, "w");
			if (file == NULL)
				g_warning("Can't open or create file %s!", path);
		}
		else
			g_warning ("Can't create directory '.glipper' in user's home directory!");
	}
	g_free(directory);
	g_free(path);
	return file;
}

void saveElem(gpointer data, gpointer histFile)
{
	GString* entry = g_string_new((gchar*)data);
	fwrite(&(entry->len), 4, 1, (FILE*)histFile);
	fputs(entry->str, (FILE*)histFile);
	g_string_free(entry, TRUE);
}

void saveHistory()
{
	FILE* histFile = writeGlipperFile("history");
	if (histFile != NULL)
	{
		fputc(g_slist_length(history), histFile);
		g_slist_foreach(history, saveElem, histFile);
		fclose(histFile);
	}
}

void readHistory()
{
	gchar* path= g_build_filename(g_get_home_dir(), ".glipper/history", NULL);
	FILE* histFile = fopen(path, "r");
	g_free(path);
	if (histFile != 0)
	{	
		int length = fgetc(histFile);
		int x;
		for (x=0; x < length; x++)
		{
			int size;
			fread(&size, 4, 1, histFile);
			gchar* data = (gchar*)g_malloc(size+1);
			fread(data, size, 1, histFile);
			data[size] = '\0';
			history = g_slist_append(history, data);
			if (!PrimaryClip.data)
				PrimaryClip.data = data;
			if (!DefaultClip.data)
				DefaultClip.data = data;
		}
	}
}

//Shows an error message dialog and outputs warning
void errorDialog(gchar* error_msg, gchar* secondaryText)
{
	GtkWidget	*dialog;

	g_warning (g_strdup_printf("%s - %s", error_msg, secondaryText));

	dialog = gtk_message_dialog_new (NULL,
		0, GTK_MESSAGE_ERROR,
		GTK_BUTTONS_OK,
		error_msg);
	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
		secondaryText);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog),
		GTK_RESPONSE_OK);

	gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);
}

const BonoboUIVerb glipper_menu_verbs [] = {
        BONOBO_UI_UNSAFE_VERB ("GlipperPreferences", showPreferences),
        BONOBO_UI_UNSAFE_VERB ("GlipperPlugins", showPluginDialog),
        BONOBO_UI_UNSAFE_VERB ("GlipperHelp", showHelp),
        BONOBO_UI_UNSAFE_VERB ("GlipperAbout", show_about),
        BONOBO_UI_VERB_END
};

void initGlipper(PanelApplet *applet)
{	
	g_static_rec_mutex_init(&mutex);
	getClipboards();
	conf = gconf_client_get_default();

	//Setup keyboard shortcut
	keybinder_init();
	keybinder_bind(gconf_client_get_string(conf, KEY_COMBINATION_KEY, NULL), keyhandler, NULL);

	if (gconf_client_get_bool(conf, SAVE_HISTORY_KEY, NULL))
		readHistory();

	mainTimeout = g_timeout_add(gconf_client_get_int(conf, CHECK_INTERVAL_KEY, NULL), checkClipboard, applet);

	gconf_client_add_dir (conf, PATH,
                         GCONF_CLIENT_PRELOAD_NONE,
                         NULL);
   
	popupKey = g_strdup(gconf_client_get_string(conf, KEY_COMBINATION_KEY, NULL));

	initPreferences(conf, applet);
	initPluginDialog(conf);
	initPlugins();
   
	//autostart plugins:
	GSList* list = gconf_client_get_list(conf, AUTOSTART_PLUGINS_KEY, GCONF_VALUE_STRING, NULL);
	for (;list != NULL; list = g_slist_next(list))
	{
		char* plugin = (char*)list->data;
		start_plugin(plugin);
	}
}

void AppletSizeAllocate (GtkWidget *widget, GtkAllocation *allocation)
{
	int iconSize;
   GdkPixbuf *pixbuf;
   
   if(allocation->height == appletSize) return;
   
   appletSize = allocation->height;
   iconSize = appletSize - 2; // Padding
   
   // We do this to prevent icon scaling
   if (iconSize <= 21)
      iconSize = 16;
   else if (iconSize <= 31)
      iconSize = 22;
   else if (iconSize <= 47)
      iconSize = 32;
      
   pixbuf = gtk_icon_theme_load_icon( gtk_icon_theme_get_default(), "glipper", iconSize, 0, NULL);
   gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
   g_object_unref(pixbuf);
}

gboolean 
glipper_applet_fill (PanelApplet *applet,
		const char  *iid,
		gpointer     data)
{	
	initGlipper(applet);
	
	if (strcmp (iid, "OAFIID:GlipperApplet") != 0)
		return FALSE;
		
	gtk_window_set_default_icon_name ("glipper");
	image = gtk_image_new();
	
	toolTip = gtk_tooltips_new ();

	gtk_tooltips_set_tip(toolTip, GTK_WIDGET(applet), g_strdup_printf(_("Glipper (%s)\nClipboardmanager"),
						popupKey), "Glipper");
	
	gtk_container_add(GTK_CONTAINER (applet), GTK_WIDGET(image));
	
	g_signal_connect (G_OBJECT (applet), 
                  "button_press_event",
                  G_CALLBACK (AppletIconClicked),
                  NULL);
	
	g_signal_connect (GTK_WIDGET (applet),
	               "size_allocate",
	               G_CALLBACK (AppletSizeAllocate),
	               NULL);
	
	panel_applet_set_flags(PANEL_APPLET (applet), PANEL_APPLET_EXPAND_MINOR);
	
	panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
                                   NULL,
                                   "GlipperApplet.xml",
                                   NULL,
                                   glipper_menu_verbs,
                                   NULL);

	gtk_widget_show_all(GTK_WIDGET(applet));
	
	return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GlipperApplet_Factory",
                             PANEL_TYPE_APPLET,
                             "Glipper Applet",
                             "0",
                             glipper_applet_fill,
                             NULL);
