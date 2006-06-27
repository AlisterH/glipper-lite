#include <gtk/gtk.h>


void
on_DefaultCheck_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void 
showPreferences                        (gpointer data);

void
on_DefaultCheck_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_clipCheck_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_closeButton_clicked                 (GtkButton       *button,
                                        gpointer         user_data);
