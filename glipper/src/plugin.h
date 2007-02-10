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

#include <Python.h>
	
typedef struct {
	char* name;
	char* descr;
	int isrunning;
	int preferences;
} plugin_info;


typedef struct menuEntry {
	struct menuEntry* next; //linked list
	char* label;
	PyObject* callback;
	char* ownerModule;
} menuEntry;

extern menuEntry menuEntryList;

extern int pluginDebug;

//Common functions for the plugin system:

int get_plugin_info(char* module, plugin_info* info);

void start_plugin(char* module);

void stop_plugin(char* module);

//Sending Events to the plugins:

void plugins_newItem();

void plugin_showPreferences(char* module);

void plugin_menu_callback(GtkMenuItem* menuItem, gpointer user_data);
