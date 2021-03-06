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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "plugin.h"
#include "main.h"

static GConfClient *global_conf;

typedef struct plugin {
	struct plugin* next; //linked list
	char* modulename; 
	PyObject* module;
	//references to the event functions:
	PyObject* newItemFunc;
	PyObject* historyChangedFunc;
	PyObject* stopFunc;
	PyObject* showPreferences;
} plugin;


static plugin pluginList; //first element in list is dummy (makes it easier to delete one special item)
static int eventsActive; //controls if the plugin events are triggered or not 

menuEntry menuEntryList; //first element is dummy

static PyObject* initCalled = NULL; //stores the module that is executing the init function 
				    //atm. Needed for menu entry registration

static int lockCount;
static PyThreadState* threadStateSave;
#define LOCK if (lockCount == 0) { PyEval_RestoreThread(threadStateSave); printf("LOCK\n"); } lockCount++;
#define UNLOCK if (lockCount == 1) { threadStateSave = PyEval_SaveThread(); printf("UNLOCK\n"); } lockCount--; g_assert(lockCount > -1);
/* call LOCK before doing any operations that concern python, and UNLOCK after that. These operations also work recursive.
 * The functions callable by the plugins are thread safe! */

////////////////////////////////////Events:///////////////////////////////////////

void plugins_newItem()
{
	if (!eventsActive)
		return;
	LOCK
	g_static_rec_mutex_lock(&mutex);
	plugin* i; 
	for (i = pluginList.next; i != NULL; i = i->next)
	{
		if (i->newItemFunc)
		{
			PyObject* args = PyTuple_New(1);
			if (args != NULL)
				PyTuple_SetItem(args, 0, PyString_FromString(history->data));
			if (!PyObject_CallObject(i->newItemFunc, args))
				PyErr_Print();
			Py_DECREF(args);
		}
	}
	g_static_rec_mutex_unlock(&mutex);
	UNLOCK
}

void plugins_historyChanged()
{
	LOCK
	plugin* i; 
	for (i = pluginList.next; i != NULL; i = i->next)
	{
		if (i->historyChangedFunc)
		{
			if (!PyObject_CallObject(i->historyChangedFunc, NULL))
				PyErr_Print();
		}
	}
	UNLOCK
}

void plugins_stop()
{
   LOCK
   plugin* i;
   
   for(i = pluginList.next; i != NULL; i = i->next)
      if(i->stopFunc)
         if(!PyObject_CallObject(i->stopFunc, NULL))
            PyErr_Print();
   
   UNLOCK
}

void plugin_menu_callback(GtkMenuItem* menuItem, gpointer user_data)
{
	LOCK
	PyObject* callback = (PyObject*)user_data;
	if (!PyObject_CallObject(callback, NULL))
		PyErr_Print();
	UNLOCK
}

///////////////////functions callable by python scripts://///////////////////////

PyObject* module_getItem(PyObject* self, PyObject* args)
{
	LOCK
	int index = PyInt_AsLong(PyTuple_GetItem(args, 0));
	g_static_rec_mutex_lock(&mutex);
	GSList* c = g_slist_nth(history, index);
	g_static_rec_mutex_unlock(&mutex);
	if (c == NULL)
	{
		UNLOCK
		Py_RETURN_NONE;
	}
	char* item = c->data;
	PyObject* res = PyString_FromString(item);
	UNLOCK
	return res;
}

PyObject* module_setItem(PyObject* self, PyObject* args)
{
	LOCK
	int index = PyInt_AsLong(PyTuple_GetItem(args, 0));
	g_static_rec_mutex_lock(&mutex);
	GSList* c = g_slist_nth(history, index);
	if (c != NULL)
	{
		free(c->data);
		char* str = g_strdup(PyString_AsString(PyTuple_GetItem(args, 1)));
		c->data = str;
		hasChanged = 1;
	}
	g_static_rec_mutex_unlock(&mutex);
	UNLOCK
	plugins_historyChanged();
	Py_RETURN_NONE;
}

PyObject* module_insertItem(PyObject* self, PyObject* args)
{
	LOCK
	eventsActive = 0;
	char* intstr = PyString_AsString(PyTuple_GetItem(args, 0));
	historyEntryActivate(NULL, intstr);
	//g_free(intstr);
	eventsActive = 1;
	UNLOCK
	Py_RETURN_NONE;
}

PyObject* module_setActiveItem(PyObject* self, PyObject* args)
{
	LOCK
	eventsActive = 0;
	int index = PyInt_AsLong(PyTuple_GetItem(args, 0));
	g_static_rec_mutex_lock(&mutex);
	GSList* c = g_slist_nth(history, index);
	g_static_rec_mutex_unlock(&mutex);
	if (c == NULL)
		Py_RETURN_NONE;
	historyEntryActivate(NULL, c->data);
	eventsActive = 1;
	UNLOCK
	Py_RETURN_NONE;
}
	
PyObject* module_clearHistory(PyObject* self, PyObject* args)
{
	g_static_rec_mutex_lock(&mutex);
	g_slist_free(history);
	history = NULL;
	hasChanged = 1;
	g_static_rec_mutex_unlock(&mutex);
	plugins_historyChanged();
	Py_RETURN_NONE;
}

PyObject* module_registerEntry(PyObject* self, PyObject* args)
{
	LOCK
	char* label = PyString_AsString(PyTuple_GetItem(args, 0));
	PyObject* callback = PyTuple_GetItem(args, 1);
	if (!callback || !PyCallable_Check(callback))
	{
		UNLOCK
		Py_RETURN_NONE;
	}
	UNLOCK

	g_static_rec_mutex_lock(&mutex);
	menuEntry* entry = malloc(sizeof(menuEntry));
	entry->label = g_strdup(label);
	entry->callback = callback;
	entry->next = menuEntryList.next;
	entry->ownerModule = initCalled;
	menuEntryList.next = entry;
	g_static_rec_mutex_unlock(&mutex);

	hasChanged = 1;
	Py_RETURN_NONE;
}

static PyMethodDef glipperFunctions[] = {
	{"getItem", module_getItem, METH_VARARGS, "returns the history item specified by the argument"},
	{"setItem", module_setItem, METH_VARARGS, "sets the history item specified by the argument"},
	{"insertItem", module_insertItem, METH_VARARGS, "adds a new item at the top of the list"},
	{"setActiveItem", module_setActiveItem, METH_VARARGS, "sets an item in the history to the current active item"},
	{"clearHistory", module_clearHistory, METH_VARARGS, "clears the whole history"},
	{"registerEntry", module_registerEntry, METH_VARARGS, "registers an entry in the menu"},
	{NULL, NULL, 0, NULL}
};


///////////////////////Functions for the plugin system://////////////////////////

void initPlugins()
{
	Py_Initialize();

	PyObject* name = PyString_FromString("sys");
	PyObject* m = PyImport_Import(name);
	Py_DECREF(name);
	PyObject* path = PyObject_GetAttrString(m, "path");
	char* searchModule = g_build_filename(g_get_home_dir(), ".glipper/plugins", NULL);
	PyList_Append(path, PyString_FromString(searchModule));
	free(searchModule);
	PyList_Append(path, PyString_FromString(PLUGINDIR));

	Py_InitModule("glipper", glipperFunctions);
	pluginList.next = NULL;
	menuEntryList.next = NULL;
	eventsActive = 1;
	lockCount = 0;
	PyEval_InitThreads();
	threadStateSave = PyEval_SaveThread(); //releases the lock
}

int get_plugin_info(char* module, plugin_info* info)
{
	LOCK
	PyObject* m = NULL;
	info->isrunning = 0;
	//search if the plugin is already started:
	plugin* i;
	for (i = pluginList.next; i != NULL; i = i->next)
		if (strcmp(i->modulename, module) == 0)
		{
			m = i->module;
			info->isrunning = 1;
			break;
		}
	//if not:
	if (m == NULL)
	{
		PyObject* name = PyString_FromString(module);
		m = PyImport_Import(name);
		Py_DECREF(name);
	}
        int res = 0;
	if (m != NULL)
	{                
		PyObject* infoFunc = PyObject_GetAttrString(m, "getInfo");
		if (infoFunc && PyCallable_Check(infoFunc))
		{
                        PyObject* result = PyObject_CallObject(infoFunc, NULL);
			if (result)
			{
				PyObject* name = PyDict_GetItemString(result, "Name");
				PyObject* descr = PyDict_GetItemString(result, "Description");
				PyObject* preferences = PyDict_GetItemString(result, "Preferences");
				info->name = PyString_AsString(name);
				info->descr = PyString_AsString(descr);
				info->preferences = PyInt_AsLong(preferences);
				Py_DECREF(result);
				res = 1;                        
			} else
				PyErr_Print();
		}
		Py_XDECREF(infoFunc);
                if (!info->isrunning)
                    Py_DECREF(m);
	}
	else
		PyErr_Print();
	UNLOCK
        return res;
}

void start_plugin(char* module)
{
	//search if the plugin was already started:
	plugin* plugins;
	for (plugins = pluginList.next; plugins != NULL; plugins = plugins->next)
		if (strcmp(plugins->modulename, module) == 0)
			return;

	LOCK
	PyObject* name = PyString_FromString(module);
	PyObject* m = PyImport_Import(name);
	Py_DECREF(name);
	if (m != NULL)
	{
		plugin* new = malloc(sizeof(plugin));
		new->next = pluginList.next;
		pluginList.next = new;
		new->modulename = malloc(strlen(module)+1);
		strcpy(new->modulename, module);
		new->module = m;
		if (PyObject_HasAttrString(m, "newItem"))
			new->newItemFunc = PyObject_GetAttrString(m, "newItem");
		else
			new->newItemFunc = NULL;
		if (PyObject_HasAttrString(m, "historyChanged"))
			new->historyChangedFunc = PyObject_GetAttrString(m, "historyChanged");
		else
			new->historyChangedFunc = NULL;
		if (PyObject_HasAttrString(m, "stop"))
			new->stopFunc = PyObject_GetAttrString(m, "stop");
		else
			new->stopFunc = NULL;
		if (PyObject_HasAttrString(m, "showPreferences"))
			new->showPreferences = PyObject_GetAttrString(m, "showPreferences");
		else
			new->showPreferences = NULL;
		if (new->newItemFunc && !PyCallable_Check(new->newItemFunc))
			new->newItemFunc = NULL;
		if (new->historyChangedFunc && !PyCallable_Check(new->historyChangedFunc))
			new->historyChangedFunc = NULL;
		printf("plugin %s started\n", module);
		PyObject* startFunction = PyObject_GetAttrString(m, "init");
		if (startFunction && PyCallable_Check(startFunction))
		{
			initCalled = m;
			if (!PyObject_CallObject(startFunction, NULL))
				PyErr_Print();
			initCalled = NULL;
		}
		Py_XDECREF(startFunction);
	}
	else
		PyErr_Print();
	UNLOCK
}

void stop_plugin(char* module)
{
	/*Notice: it seems that we can't stop the threads that were started by a plugin,
	  so the plugin has to stop them itself. */
	LOCK
	plugin* i = &pluginList;
	while (i->next != NULL)
	{
			plugin* c = i->next;
			if (strcmp(c->modulename, module) == 0)
			{
				//search for all belonging menu entries and delete them:
				if (menuEntryList.next != NULL)
				{
					menuEntry* it;
					for (it = &menuEntryList; it->next != NULL;)
						if (it->next->ownerModule == c->module)
						{
							menuEntry* tmp = it->next;
							it->next = it->next->next;
							free(tmp->label);
							free(tmp);
						} else
							it = it->next;
					hasChanged = 1;
				}

				Py_DECREF(c->module);
				Py_XDECREF(c->newItemFunc);
				Py_XDECREF(c->historyChangedFunc);
				Py_XDECREF(c->showPreferences);
				free(c->modulename);
				i->next = c->next;
				free(c);
				printf("plugin %s stopped\n", module);
				break;
			}
			i = i->next;
	}
	UNLOCK
}

void plugin_showPreferences(char* module)
{
	LOCK
	//search if the plugin is already started:
	plugin* i;
	for (i = pluginList.next; i != NULL; i = i->next)
		if (strcmp(i->modulename, module) == 0)
			break;
	//if yes:
	if (i != NULL)
	{
		if (i->showPreferences != NULL)
			if (!PyObject_CallObject(i->showPreferences, NULL))
				PyErr_Print();
	}
	else //if not, we have to load the module:
	{
		PyObject* m;
		PyObject* name = PyString_FromString(module);
		m = PyImport_Import(name);
		Py_DECREF(name);
		if (m != NULL)
		{
			PyObject* prefFunc = PyObject_GetAttrString(m, "showPreferences");
			if (prefFunc && PyCallable_Check(prefFunc))
				if (!PyObject_CallObject(prefFunc, NULL))
					PyErr_Print();
			Py_XDECREF(prefFunc);
			Py_DECREF(m);
		} else
			PyErr_Print();
	}
	UNLOCK
}
