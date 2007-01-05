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
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "plugin.h"
#include "main.h"

typedef struct plugin {
	struct plugin* next; //linked list
	char* modulename; 
	PyObject* module;
	//references to the event functions:
	PyObject* newItemFunc;
	//TODO: add more
} plugin;

static plugin pluginList; //first element in list is dummy (makes it easier to delete one special item)
static int eventsActive; //controls if the plugin events are triggered or not 

int pluginDebug = 1;

////////////////////////////////////Events:///////////////////////////////////////

void plugins_newItem()
{
	if (!eventsActive)
		return;
	plugin* i; 
	for (i = pluginList.next; i != NULL; i = i->next)
	{
		if (i->newItemFunc)
		{
			PyObject* args = PyTuple_New(1);
			if (args != NULL)
				PyTuple_SetItem(args, 0, PyString_FromString(history->data));
			PyObject_CallObject(i->newItemFunc, args);	
			Py_DECREF(args);
		}
	}
}

///////////////////functions callable by python scripts://///////////////////////

PyObject* module_getItem(PyObject* self, PyObject* args)
{
	int index = PyInt_AsLong(PyTuple_GetItem(args, 0));
	GSList* c = g_slist_nth(history, index);
	if (c == NULL)
		return NULL;
	char* item = c->data;
	return PyString_FromString(item);
}

PyObject* module_setItem(PyObject* self, PyObject* args)
{
	int index = PyInt_AsLong(PyTuple_GetItem(args, 0));
	GSList* c = g_slist_nth(history, index);
	if (c != NULL)
	{
		free(c->data);
		char* intstr = PyString_AsString(PyTuple_GetItem(args, 1));
		char* str = malloc(strlen(intstr)+1);
		strcpy(str, intstr);
		c->data = str;
		hasChanged = 1;
	}
	return NULL;
}
	
PyObject* module_insertItem(PyObject* self, PyObject* args)
{
	eventsActive = 0;
	char* intstr = PyString_AsString(PyTuple_GetItem(args, 0));
	insertInHistory(intstr);
	eventsActive = 1;
	return NULL;
}

PyObject* module_setActiveItem(PyObject* self, PyObject* args)
{
	eventsActive = 0;
	int index = PyInt_AsLong(PyTuple_GetItem(args, 0));
	GSList* c = g_slist_nth(history, index);
	if (c == NULL)
		return NULL;
	historyEntryActivate(NULL, c->data);
	eventsActive = 1;
	return NULL;
}
	
PyObject* module_clearHistory(PyObject* self, PyObject* args)
{
	g_slist_free(history);
	history = NULL;
	hasChanged = 1;
	return NULL;
}

static PyMethodDef glipperFunctions[] = {
	{"getItem", module_getItem, METH_VARARGS, "returns the history item specified by the argument"},
	{"setItem", module_setItem, METH_VARARGS, "sets the history item specified by the argument"},
	{"insertItem", module_insertItem, METH_VARARGS, "adds a new item at the top of the list"},
	{"setActiveItem", module_setActiveItem, METH_VARARGS, "sets an item in the history to the current active item"},
	{"clearHistory", module_clearHistory, METH_VARARGS, "clears the whole history"},
	{NULL, NULL, 0, NULL}
};


///////////////////////Functions for the plugin system://////////////////////////

void init()
{
	static int pyInit = 0;
	if (!pyInit)
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
		pyInit = 1;
		eventsActive = 1;
	}
}

int get_plugin_info(char* module, plugin_info* info)
{
	init();
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
			PyObject* name = PyDict_GetItemString(result, "Name");
			PyObject* descr = PyDict_GetItemString(result, "Description");
			info->name = PyString_AsString(name);
			info->descr = PyString_AsString(descr);
			Py_DECREF(name);
			Py_DECREF(descr);
			Py_DECREF(result);
			res = 1;                        
		}
		Py_DECREF(infoFunc);
                if (!info->isrunning)
                    Py_DECREF(m);
	}
	else
		if (pluginDebug)
			PyErr_Print();
        return res;
}

void start_plugin(char* module)
{
	init();
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
		new->newItemFunc = PyObject_GetAttrString(m, "newItem");
		//TODO: other event functions
		if (new->newItemFunc && !PyCallable_Check(new->newItemFunc))
			new->newItemFunc = NULL;
		printf("plugin %s started\n", module);
		PyObject* startFunction = PyObject_GetAttrString(m, "start");
		if (startFunction && PyCallable_Check(startFunction))
			PyObject_CallObject(startFunction, NULL);
		Py_XDECREF(startFunction);
	}
	else
		if (pluginDebug)
			PyErr_Print();
}

void stop_plugin(char* module)
{
	plugin* i = &pluginList;
	while (i->next != NULL)
	{
			plugin* c = i->next;
			if (strcmp(c->modulename, module) == 0)
			{
				Py_DECREF(c->module);
				Py_DECREF(c->newItemFunc);
				free(c->modulename);
				i->next = c->next;
				free(c);
				printf("plugin %s stopped\n", module);
				break;
			}
			i = i->next;
	}
}
