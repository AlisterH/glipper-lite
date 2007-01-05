#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#define PATH "/apps"
#define KEY_PREFIX PATH "/glipper"
#define MAX_ELEMENTS_KEY KEY_PREFIX "/max_elements"
#define MAX_ITEM_LENGTH_KEY KEY_PREFIX "/max_item_length"
#define USE_PRIMARY_CLIPBOARD_KEY KEY_PREFIX "/use_primary_clipboard"
#define USE_DEFAULT_CLIPBOARD_KEY KEY_PREFIX "/use_default_clipboard"
#define MARK_DEFAULT_ENTRY_KEY KEY_PREFIX "/mark_default_entry"
#define SAVE_HISTORY_KEY KEY_PREFIX "/save_history"
#define CHECK_INTERVAL_KEY KEY_PREFIX "/check_interval"
#define KEY_COMBINATION_KEY KEY_PREFIX "/key_combination"

void
on_DefaultCheck_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void 
showPreferences                        (gpointer data);

void
on_primaryCheck_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_defaultCheck_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_closeButton_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_helpButton_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_itemLength_value_changed            (GtkSpinButton       *spinButton,
                                        gpointer         user_data);

void
on_historyLength_value_changed            (GtkSpinButton       *spinButton,
                                        gpointer         user_data);
    
void
on_markDefaultCheck_toggled            (GtkToggleButton *toggleButton,
                                        gpointer         user_data);

void
on_saveHistCheck_toggled            (GtkToggleButton *toggleButton,
                                        gpointer         user_data);

void
on_keyCombEntry_changed            (GtkEntry *entry,
                                        gpointer         user_data);
                                        
void max_item_length_key_changed_callback(GConfClient *client,
		    guint        cnxn_id,
		    GConfEntry  *entry,
		    gpointer     user_data);

void max_elements_key_changed_callback(GConfClient *client,
		    guint        cnxn_id,
		    GConfEntry  *entry,
		    gpointer     user_data);

void use_primary_clipboard_key_changed_callback(GConfClient *client,
		    guint        cnxn_id,
		    GConfEntry  *entry,
		    gpointer     user_data);
										
void use_default_clipboard_key_changed_callback(GConfClient *client,
		    guint        cnxn_id,
		    GConfEntry  *entry,
		    gpointer     user_data);
										
void mark_default_entry_key_changed_callback(GConfClient *client,
		    guint        cnxn_id,
		    GConfEntry  *entry,
		    gpointer     user_data);
										
void save_history_key_changed_callback(GConfClient *client,
		    guint        cnxn_id,
		    GConfEntry  *entry,
		    gpointer     user_data);
										
void key_combination_key_changed_callback(GConfClient *client,
		    guint        cnxn_id,
		    GConfEntry  *entry,
		    gpointer     user_data);
