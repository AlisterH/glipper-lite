#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include "preferences.h"
#include "main.h"
#include "utils/glipper-i18n.h"

#define UI_XML_FILE "glipper-properties.ui"

GtkWidget* historyLength;
GtkWidget* itemLength;
GtkWidget* primaryCheck;
GtkWidget* defaultCheck;
GtkWidget* markDefaultCheck;
GtkWidget* saveHistCheck;
GtkWidget* keyCombEntry;
GtkWidget* applyButton;
GtkWidget* prefWin;

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

void showPreferences(gpointer data)
{
	char* ui_file;

	//Load interface from ui file
	ui_file = g_build_filename(UI_DIR, UI_XML_FILE, NULL);


	GError* error = NULL;
		GtkBuilder* builder = gtk_builder_new();
		if (!gtk_builder_add_from_file (builder, ui_file, &error))
	{
			errorDialog(_("Could not load the preferences interface"), error->message);
			//    No, this only prints the message, doesn't show a window
			//    g_warning ("Couldn't load builder file: %s", error->message);
			g_error_free(error);
		return;
	}


	g_free (ui_file);

	//Get the widgets that we will need
	historyLength = GTK_WIDGET (gtk_builder_get_object (builder, "historyLength"));
	itemLength = GTK_WIDGET (gtk_builder_get_object (builder, "itemLength"));
	primaryCheck = GTK_WIDGET (gtk_builder_get_object (builder, "primaryCheck"));
	defaultCheck = GTK_WIDGET (gtk_builder_get_object (builder, "defaultCheck"));
	markDefaultCheck = GTK_WIDGET (gtk_builder_get_object (builder, "markDefaultCheck"));
	saveHistCheck = GTK_WIDGET (gtk_builder_get_object (builder, "saveHistCheck"));
	keyCombEntry = GTK_WIDGET (gtk_builder_get_object (builder, "keyComb"));
	applyButton = GTK_WIDGET (gtk_builder_get_object (builder, "applyButton"));
	prefWin = GTK_WIDGET (gtk_builder_get_object (builder, "preferences-dialog"));


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

	//Show preferences dialog

	setWidgets();
	gtk_widget_show_all(prefWin);

	//free the gtkbuilder data
	g_object_unref(builder);
}
