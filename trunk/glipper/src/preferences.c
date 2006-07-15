#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glade/glade.h>
#include "preferences.h"
#include "main.h"
#include "utils/glipper-i18n.h"

#define GLADE_XML_FILE "glipper-properties.glade"

GtkWidget* historyLength;
GtkWidget* itemLength;
GtkWidget* primaryCheck;
GtkWidget* defaultCheck;
GtkWidget* markDefaultCheck;
GtkWidget* saveHistCheck;
GtkWidget* keyCombEntry;
GtkWidget* applyButton;
GtkWidget* prefWin;
GtkWidget* helpButton;

//Sets initial state of widgets
void setWidgets()
{
	gtk_spin_button_set_value((GtkSpinButton*)historyLength, maxElements);
	gtk_spin_button_set_value((GtkSpinButton*)itemLength, maxItemLength);
	gtk_toggle_button_set_active((GtkToggleButton*)primaryCheck, usePrimary);
	gtk_toggle_button_set_active((GtkToggleButton*)defaultCheck, useDefault);
	gtk_toggle_button_set_active((GtkToggleButton*)markDefaultCheck, markDefault);
	gtk_toggle_button_set_active((GtkToggleButton*)saveHistCheck, weSaveHistory);
	gtk_entry_set_text((GtkEntry*)keyCombEntry, keyComb);
	on_clipCheck_toggled(NULL, NULL); //can make markDefaultCheck possibly unsensitive
}

//Set markDefaultCheck checkbox (ctrl+c in blue) active or not
void
on_clipCheck_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	//Only permit to distinguish ctrl+c entry if both types of clipboard are present
	if (!(gtk_toggle_button_get_active((GtkToggleButton*)primaryCheck) && 
	  gtk_toggle_button_get_active((GtkToggleButton*)defaultCheck)))
		gtk_widget_set_sensitive(markDefaultCheck, FALSE);
	else
		gtk_widget_set_sensitive(markDefaultCheck, TRUE);
}

//Save preferences when window is closed via apply button
void
on_applyButton_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	unbindKey();
	maxElements = gtk_spin_button_get_value_as_int((GtkSpinButton*)historyLength); 
	maxItemLength = gtk_spin_button_get_value_as_int((GtkSpinButton*)itemLength); 
	usePrimary = gtk_toggle_button_get_active((GtkToggleButton*)primaryCheck);
	useDefault = gtk_toggle_button_get_active((GtkToggleButton*)defaultCheck);
	markDefault = gtk_toggle_button_get_active((GtkToggleButton*)markDefaultCheck);
	weSaveHistory = gtk_toggle_button_get_active((GtkToggleButton*)saveHistCheck);
	keyComb = g_strdup(gtk_entry_get_text((GtkEntry*)keyCombEntry));
	savePreferences();
	applyPreferences();
	gtk_widget_destroy(prefWin);
}

on_helpButton_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
#ifndef DISABLE_GNOME
    help("preferences");
#endif /*DISABLE_GNOME*/

#ifdef DISABLE_GNOME
    errorDialog(_("Help support is not compiled in."), _("To see the documentation, consult the glipper website or compile glipper with GNOME support (see README file)."));
#endif /*DISABLE_GNOME*/
}

void showPreferences(gpointer data)
{
	char* glade_file;
	GladeXML* gladeWindow;

	//Load interface from glade file
	glade_file = g_build_filename(GLADEDIR, GLADE_XML_FILE, NULL);

	gladeWindow = glade_xml_new(glade_file, "preferences-dialog", NULL);

	//In case we cannot load glade file
	if (gladeWindow == NULL)
	{
		errorDialog(_("Could not load the preferences interface"), glade_file);
		g_free (glade_file);
		return;
	}

	g_free (glade_file);

	//Get the widgets that we will need
	historyLength = glade_xml_get_widget(gladeWindow, "historyLength");
	itemLength = glade_xml_get_widget(gladeWindow, "itemLength");
	primaryCheck = glade_xml_get_widget(gladeWindow, "primaryCheck");
	defaultCheck = glade_xml_get_widget(gladeWindow, "defaultCheck");
	markDefaultCheck = glade_xml_get_widget(gladeWindow, "markDefaultCheck");
	saveHistCheck = glade_xml_get_widget(gladeWindow, "saveHistCheck");
	keyCombEntry = glade_xml_get_widget(gladeWindow, "keyComb");
	applyButton = glade_xml_get_widget(gladeWindow, "applyButton");
	helpButton = glade_xml_get_widget(gladeWindow, "helpButton");
	prefWin = glade_xml_get_widget(gladeWindow, "preferences-dialog");

	//Connect signals to handlers
	g_signal_connect_after ((gpointer) primaryCheck, "toggled",
		G_CALLBACK (on_clipCheck_toggled),
                          NULL);
	g_signal_connect_after ((gpointer) defaultCheck, "toggled",
		G_CALLBACK (on_clipCheck_toggled),
		NULL);
	g_signal_connect ((gpointer) applyButton, "clicked",
		G_CALLBACK (on_applyButton_clicked),
		NULL);
	g_signal_connect ((gpointer) helpButton, "clicked",
		G_CALLBACK (on_helpButton_clicked),
		NULL);

	//Show preferences dialog
	setWidgets();
	gtk_widget_show_all(prefWin);

	//free the glade data
	g_object_unref(gladeWindow);
}
