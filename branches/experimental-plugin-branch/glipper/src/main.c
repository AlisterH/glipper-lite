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
				 
#include <gtk/gtk.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "preferences.h"
#include "utils/eggtrayicon.h"
#include "utils/glipper-i18n.h"
#include "utils/keybinder.h"
#include "plugin.h"

#ifndef DISABLE_GNOME
#include <libgnome/gnome-help.h>
#endif /*DISABLE_GNOME*/

#define CHECK_INTERVAL 500 //The interval between the clipboard checks (ms)

#define PLUGINS

//Preferences variables
int maxElements = 20; //Amount of elements in history
int maxItemLength = 35; //Length of one history entry
gboolean usePrimary = TRUE; //use Primary Clipboard
gboolean useDefault = TRUE;; //use Default Clipboard
gboolean markDefault = TRUE; //whether default entry should be tagged
gboolean weSaveHistory = TRUE; //whether history should be saved
char* keyComb;

GtkClipboard* PrimaryCl;
GtkClipboard* DefaultCl;
gchar* lastPr = NULL;
gchar* lastDf = NULL;
GSList* history = NULL;
GtkWidget* historyMenu = NULL;
GtkWidget* popupMenu;
GtkWidget* TrayIcon;
GtkTooltips* toolTip; //The trayicon's tooltip
GtkWidget* eventbox; //The trayicon's eventbox
gint mainTimeout;
int hasChanged = 1;

void errorDialog(gchar* error_msg, gchar* secondaryText);

void getClipboards()
{
	/*There exists two different Clipboards:
	  PrimaryClipboard: Marked with mouse
	  DefaultClipboard: copied with Strg+C
	*/
	DefaultCl = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	PrimaryCl = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
}

void deleteHistory(GtkMenuItem* menuItem, gpointer user_data)
{
	hasChanged = 1;
	g_slist_free(history);
	history = NULL;
}

void historyEntryActivate(GtkMenuItem* menuItem, gpointer user_data);

//add history entry to menu:
GtkWidget* addHistMenuItem(gchar* item)
{
	static GtkTooltips* toolTip; //The tooltip for bold item

	if (toolTip == NULL) toolTip = gtk_tooltips_new();

	//we have to cut the string to "maxItemLength" characters:
	GString* ellipseData = g_string_new(item);
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
	if (markDefault && useDefault && usePrimary)
		if ((lastDf!=NULL)&&(g_ascii_strcasecmp(lastDf, item)==0))
		{
			GtkLabel* label = (GtkLabel*)gtk_bin_get_child((GtkBin*)MenuItem); 
			gchar* temp = g_strdup_printf("<b>%s</b>", ellipseData->str);
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
		pixbuf = gdk_pixbuf_new_from_file(PIXMAPDIR"/glipper.png", &pix_error);

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
			g_strdup_printf(_("Glipper - Clipboardmanager  (%s)"), keyComb));

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
		GtkWidget* firstItem = addHistMenuItem(temp->data);
		//addHistMenuItem(temp->data);

		//Add description tooltip to first item
		gtk_tooltips_set_tip(toolTip, firstItem, _("This is the last element to be copied.\nIt can be pasted with the middle mouse button."), "Glipper");

		if (temp->next != NULL)
		{
			gtk_menu_append((GtkMenu*)historyMenu, gtk_separator_menu_item_new());
			while ((temp = g_slist_next(temp)) != NULL)
				addHistMenuItem(temp->data);
		}
	}

	//Add the "clear" button at the bottom of the menu
	gtk_menu_append((GtkMenu*)historyMenu, gtk_separator_menu_item_new());

	GtkWidget* deleteAll =
		gtk_image_menu_item_new_from_stock(GTK_STOCK_CLEAR, NULL);
	g_signal_connect(G_OBJECT(deleteAll), "activate", G_CALLBACK(deleteHistory), NULL);
	gtk_menu_append((GtkMenu*)historyMenu, deleteAll);

	gtk_widget_show_all(historyMenu);
}


void saveHistory();

void insertInHistory(gchar* content)
{
	if ((history == NULL) ||
	  (g_ascii_strcasecmp(content, (gchar*)history->data)!=0))
	{
		//g_print(content);
		history = g_slist_prepend(history, g_strdup(content));
		hasChanged = 1;
		//We look whether the same entry still exists in the history and delete it:
		GSList* temp = history;
		while ((temp->next!=NULL)&&(temp->next->data!=NULL)&&
		  (g_ascii_strcasecmp((gchar*)temp->next->data, content)!=0))
		  	temp = temp->next;
		//If the same entry was found:
		if (temp->next!=NULL)
		{
			GSList* dummy = temp->next;
			temp->next = temp->next->next;
			g_slist_free_1(dummy);
		}
		//We shorten the history if it gets longer than "maxElements":
		GSList* deleteElement = g_slist_nth(history, maxElements-1);
		if (deleteElement != NULL)
		{
			g_slist_free(deleteElement->next);
			deleteElement->next = NULL;
		}
#ifdef PLUGINS
		plugins_newItem();
#endif
	}
	if (weSaveHistory)
		saveHistory();
}

void processContent(gchar* newContent, gchar** lastContent, GtkClipboard* Clipboard)
{
	if (newContent == NULL)
	{
		if (*lastContent != NULL)
			gtk_clipboard_set_text(Clipboard, *lastContent, -1);
	} else {
		if ((*lastContent==NULL)||
		  ((*lastContent!=NULL)&&(g_ascii_strcasecmp(newContent, *lastContent)!=0)))
		{
			insertInHistory(newContent);
			if (*lastContent!=NULL)
				g_free(*lastContent);
			*lastContent = newContent;
		}
		else
			g_free(newContent);
	}
}

gboolean checkClipboard(gpointer data)
{
	g_source_remove(mainTimeout);
	if (usePrimary)
	{
		void* infos[] = {PrimaryCl, lastPr};
		gchar* newContentPr = gtk_clipboard_wait_for_text(PrimaryCl);
		processContent(newContentPr, &lastPr, PrimaryCl);
	}
	if (useDefault)
	{
		void* infos[] = {DefaultCl, lastDf};
		gchar* newContentCl = gtk_clipboard_wait_for_text(DefaultCl);
		processContent(newContentCl, &lastDf, DefaultCl);
	}
	mainTimeout = g_timeout_add(500, checkClipboard, NULL);
	return 1;
}

void TrayIconClicked(GtkWidget* widget, GdkEventButton *event, gpointer user_data)
{
	if (event->type != GDK_BUTTON_PRESS)
		return;
	switch (event->button)
	{
		case 1:
			if (hasChanged)
				createHistMenu();
			hasChanged = 0;
			gtk_menu_popup ((GtkMenu*)historyMenu, NULL, NULL, NULL, NULL,
				1, gtk_get_current_event_time());
			break;
		case 3:
			gtk_menu_popup((GtkMenu*)popupMenu, NULL, NULL, NULL, NULL, 
				1, gtk_get_current_event_time());
			break;
	}
}

void historyEntryActivate(GtkMenuItem* menuItem, gpointer user_data)
{
	if (usePrimary)
		gtk_clipboard_set_text(PrimaryCl, (gchar*)user_data, -1);
	if (useDefault)
		gtk_clipboard_set_text(DefaultCl, (gchar*)user_data, -1);
	checkClipboard(NULL);
	hasChanged = 1;
}

#ifndef DISABLE_GNOME
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

void showHelp(gpointer data)
{
	help(NULL);
}
#endif /*DISABLE_GNOME*/


void createTrayIcon()
{
	GdkPixbuf* pixbuf, *scaled;
	GtkWidget* tray_icon_image;
	GError* pix_error = NULL;

	//Load trayicon
	TrayIcon = (GtkWidget*)egg_tray_icon_new("GLIPPER");
	pixbuf = gdk_pixbuf_new_from_file(PIXMAPDIR"/glipper.png", &pix_error);

	//In case we cannot load icon, display error message and exit
	if (pix_error != NULL)
	{
		errorDialog(pix_error->message, _("Cannot load icon. Is the software properly installed ?"));
		exit(1);
	}

	tray_icon_image = gtk_image_new_from_pixbuf(pixbuf);
	gdk_pixbuf_unref(pixbuf);

	//create eventbox:
	eventbox = gtk_event_box_new();

	//Add description tooltip to the icon
	toolTip = gtk_tooltips_new();
	gtk_tooltips_set_tip(toolTip, eventbox, g_strdup_printf(_("Glipper (%s)\nClipboardmanager"),
		keyComb), "Glipper");

	//connect and show everything:
	gtk_container_add(GTK_CONTAINER(eventbox), tray_icon_image);
	gtk_container_add (GTK_CONTAINER(TrayIcon), eventbox);
	gtk_widget_show_all(GTK_WIDGET(TrayIcon));
	g_signal_connect_swapped(G_OBJECT(eventbox), "button-press-event", 
							 G_CALLBACK(TrayIconClicked), NULL);
}

void show_about(gpointer data)
{
	gchar* authors[] = {"Sven Rech <svenrech@gmx.de>", NULL};

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

	GError* pix_error = NULL;
	gtk_show_about_dialog(NULL, 
		"authors", authors,
		"copyright", "Copyright © 2006 Sven Rech", 
		"license", license,
		"name", "Glipper",
		"comments", _("Clipboardmanager for Gnome"),
		"logo", gdk_pixbuf_new_from_file(PIXMAPDIR"/glipper.png", &pix_error),
		"version", VERSION,
		NULL);
}
//Creates context menu that we popup on right click on the trayicon
void createPopupMenu()
{
	//Create new popup menu
	popupMenu = gtk_menu_new();

	//Create widgets to be plced in the popup menu
	GtkWidget* quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	g_signal_connect(G_OBJECT(quit), "activate", G_CALLBACK(gtk_main_quit), NULL);

	GtkWidget* about = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
	g_signal_connect(G_OBJECT(about), "activate", G_CALLBACK(show_about), NULL);

#ifndef DISABLE_GNOME
	GtkWidget* help = gtk_image_menu_item_new_from_stock(GTK_STOCK_HELP, NULL);
	g_signal_connect(G_OBJECT(help), "activate", G_CALLBACK(showHelp), NULL);
#endif /*DISABLE_GNOME*/

	GtkWidget* preferences = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, NULL);
	g_signal_connect(G_OBJECT(preferences), "activate", G_CALLBACK(showPreferences), NULL);

	//Add the widgets to the menu
	gtk_menu_append((GtkMenu*)popupMenu, preferences);
#ifndef DISABLE_GNOME
	gtk_menu_append((GtkMenu*)popupMenu, help);
#endif /*DISABLE_GNOME*/
	gtk_menu_append((GtkMenu*)popupMenu, about);
	gtk_menu_append((GtkMenu*)popupMenu, gtk_separator_menu_item_new());
	gtk_menu_append((GtkMenu*)popupMenu, quit);

	gtk_widget_show_all(popupMenu);
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
		if (mkdir(directory, S_IRWXU)==0)  //Trys to create the folder if open failes
		{
			file = fopen(path, "w");
			if (file == NULL)
				g_warning("Can't open or create file %s!", file);
		}
		else
			g_warning ("Can't create directory '.glipper' in user's home directory!");
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
		}
	}
}

void readPreferences()
{
	gchar* path= g_build_filename(g_get_home_dir(), ".glipper/prefs", NULL);
	FILE* prefFile = fopen(path, "r");
	g_free(path);
	free(keyComb);
	if (prefFile != 0)
	{
		fread(&maxElements, sizeof(maxElements), 1, prefFile);
		fread(&maxItemLength, sizeof(maxItemLength), 1, prefFile);
		fread(&usePrimary, sizeof(usePrimary), 1, prefFile);
		fread(&useDefault, sizeof(useDefault), 1, prefFile);
		fread(&markDefault, sizeof(markDefault), 1, prefFile);
		fread(&weSaveHistory, sizeof(weSaveHistory), 1, prefFile);
		size_t len;
		fread(&len, sizeof(size_t), 1, prefFile);
		if (!feof(prefFile))
		{
			keyComb = malloc(len+1);
			fgets(keyComb, len+1, prefFile);
			fclose(prefFile);
			return;
		}
		fclose(prefFile);
	}
	keyComb = malloc(strlen("<Ctrl><Alt>c")+1);
	strcpy(keyComb, "<Ctrl><Alt>c");
}

/*Makes sure, that the current preferences are used:*/
void applyPreferences()
{
	hasChanged=1; 
	GSList* deleteElement = g_slist_nth(history, maxElements-1);
	if (deleteElement != NULL)
	{
		g_slist_free(deleteElement->next);
		deleteElement->next = NULL;
	}
	keybinder_bind(keyComb, keyhandler, NULL);
	gtk_tooltips_set_tip(toolTip, eventbox, g_strdup_printf(_("Glipper (%s)\nClipboardmanager"),
		keyComb), "Glipper");
}

void savePreferences()
{
	FILE* prefFile = writeGlipperFile("prefs");
	if (prefFile != 0)
	{
		fwrite(&maxElements, sizeof(maxElements), 1, prefFile);
		fwrite(&maxItemLength, sizeof(maxItemLength), 1, prefFile);
		fwrite(&usePrimary, sizeof(usePrimary), 1, prefFile);
		fwrite(&useDefault, sizeof(useDefault), 1, prefFile);
		fwrite(&markDefault, sizeof(markDefault), 1, prefFile);
		fwrite(&weSaveHistory, sizeof(weSaveHistory), 1, prefFile);
		size_t len = strlen(keyComb);
		fwrite(&len, sizeof(size_t), 1, prefFile);
		fputs(keyComb, prefFile);
		fclose(prefFile);
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

void unbindKey()
{
	keybinder_unbind(keyComb, keyhandler);
	free(keyComb);
	keyComb = NULL;
}

int main(int argc, char *argv[])
{
	//gettext configuration
	setlocale(LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, GLIPPERLOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	//Init GTK+ (and optionally GNOME libs)
#ifndef DISABLE_GNOME
	gnome_program_init("glipper", VERSION, NULL, argc, argv,
		NULL, NULL, NULL);
#endif /*DISABLE_GNOME*/
	gtk_init (&argc, &argv);

	getClipboards();
	readPreferences();

	//Setup keyboard shortcut
	keybinder_init();
	keybinder_bind(keyComb, keyhandler, NULL);

	createTrayIcon();
	createPopupMenu();

	if (weSaveHistory)
		readHistory();

	mainTimeout = g_timeout_add(CHECK_INTERVAL, checkClipboard, NULL);
	start_plugin("newline");
	gtk_main ();

	unbindKey();

	return 0;
}
