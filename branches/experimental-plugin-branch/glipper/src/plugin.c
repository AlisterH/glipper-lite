#include <Python.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "plugin.h"
#include "main.h"

static struct plugin {
	plugin* next; //linked list
	char* modulename; 
	PyObject* module;
	//references to the event functions:
	PyObject* newItemFunc;
	//TODO: add more
}

static PyMethodDef glipperFunctions[] = {
	{"getItem", module_getItem, METH_VARARGS, "Returns the history item specified by the argument"},
	{"setItem", module_setItem, METH_VARARGS, "Sets the history item specified by the argument"},
	{"clearHistory", module_clearHistory, METH_VARARGS, "Clears the whole history"},
	{NULL, NULL, 0, NULL}
};

int pyInit = 0;
plugin pluginList; //first element in list is dummy (makes it easier to delete one special item)

///////////////////////Functions for the plugin system://////////////////////////

void init()
{
	if (!pyInit)
	{
		Py_Initialize();
		Py_InitModule("glipper", glipperFunctions);
		pyInit = 1;
	}
}

struct plugin_info get_plugin_info(char* module)
{
	init();
	PyObject* name = PyString_FromString(module);
	PyObject* m = PyImport_Import(name);
	Py_DECREF(name);
	if (m != NULL)
	{
		plugin_info info;
		PyObject* infoFunc = PyObject_GetAttrString(m, "getInfo");
		if (infoFunc && PyCallable_Check(infoFunc))
		{
			PyObject* result = PyObject_CallObject(infoFunc, NULL);
			PyObject* name = PyDict_GetItemString(result, "Name");
			PyObject* descr = PyDict_GetItemString(result, "Description");
			info.name = PyString_AsString(name);
			info.descr = PyString_AsString(descr);
			Py_DECREF(name);
			Py_DECREF(descr);
			Py_DECREF(result);
		}
		Py_DECREF(infoFunc);
		Py_DECREF(m);
		return info;
	}
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
		new->modulename = module;
		new->module = m;
		new->newItemFunc = PyObject_GetAttrString(m, "newItem");
		//TODO: other event functions
		if (!PyCallable_Check(new->newItemFunc))
			new->newItemFunc = NULL;
	}
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
			}
			i = i->next;
	}
}

////////////////////////////////////Events:///////////////////////////////////////

void plugins_newItem()
{
	if (pluginList.next == NULL)
		return;
	plugin* i; 
	for (i = pluginList.next; i->next != NULL; i = i->next)
	{
		PyObject* args = PyTuple_New(1);
		if (args != NULL)
			PyTuple_SetItem(args, 0, PyString_FromString(history->data));
		PyObject_CallObject(i->newItemFunc, args);	
		Py_DECREF(args);
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

void module_setItem(PyObject* self, PyObject* args)
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
}
	
void module_clearHistory(PyObject* self, PyObject* args)
{
	g_slist_free(history);
	history = NULL;
	hasChanged = 1;
}
