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
GtkWidget* closeButton;
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
on_closeButton_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	maxElements = gtk_spin_button_get_value_as_int((GtkSpinButton*)historyLength); 
	maxItemLength = gtk_spin_button_get_value_as_int((GtkSpinButton*)itemLength); 
	usePrimary = gtk_toggle_button_get_active((GtkToggleButton*)primaryCheck);
	useDefault = gtk_toggle_button_get_active((GtkToggleButton*)defaultCheck);
	markDefault = gtk_toggle_button_get_active((GtkToggleButton*)markDefaultCheck);
	weSaveHistory = gtk_toggle_button_get_active((GtkToggleButton*)saveHistCheck);
	savePreferences();
	applyPreferences();
	gtk_widget_destroy(prefWin);
}

void giveWindow(void)
{
	char		*glade_file;
	GladeXML	*prefWindow;

	//Load interface from glade file
	glade_file = g_build_filename(GLADEDIR, GLADE_XML_FILE, NULL);

	prefWindow = glade_xml_new(glade_file, "preferences-dialog", NULL);

	//In case we cannot load glade file
	if (prefWindow == NULL)
	{
		errorDialog(_("Could not load the preferences interface"), glade_file);
		g_free (glade_file);
		return;
	}

	g_free (glade_file);

	//Get the widgets that we will need
	historyLength = glade_xml_get_widget(prefWindow, "historyLength");
	itemLength = glade_xml_get_widget(prefWindow, "itemLength");
	primaryCheck = glade_xml_get_widget(prefWindow, "primaryCheck");
	defaultCheck = glade_xml_get_widget(prefWindow, "defaultCheck");
	markDefaultCheck = glade_xml_get_widget(prefWindow, "markDefaultCheck");
	saveHistCheck = glade_xml_get_widget(prefWindow, "saveHistCheck");
	closeButton = glade_xml_get_widget(prefWindow, "closeButton");
	prefWin = glade_xml_get_widget(prefWindow, "preferences-dialog");

	//Connect signals to handlers
	g_signal_connect_after ((gpointer) primaryCheck, "toggled",
		G_CALLBACK (on_clipCheck_toggled),
                          NULL);
	g_signal_connect_after ((gpointer) defaultCheck, "toggled",
		G_CALLBACK (on_clipCheck_toggled),
		NULL);
	g_signal_connect ((gpointer) closeButton, "clicked",
		G_CALLBACK (on_closeButton_clicked),
		NULL);

	//Show preferences dialog
	gtk_widget_show_all(glade_xml_get_widget (prefWindow, "preferences-dialog"));

	setWidgets();
}
