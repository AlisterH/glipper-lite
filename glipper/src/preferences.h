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

#pragma once
	
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

void showPreferences (gpointer data);

void initPreferences(GConfClient* conf);
