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

plugin pluginList; //first element in list is dummy (makes it easier to delete one special item)

////////////////////////////////////Events:///////////////////////////////////////

void plugins_newItem()
{
	plugin* i; 
	for (i = pluginList.next; i != NULL; i = i->next)
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
	
PyObject* module_newItem(PyObject* self, PyObject* args)
{
	char* intstr = PyString_AsString(PyTuple_GetItem(args, 0));
	char* str = malloc(strlen(intstr)+1);
	strcpy(str, intstr);
	history = g_slist_prepend(history, str);
	hasChanged = 1;
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
	{"getItem", module_getItem, METH_VARARGS, "Returns the history item specified by the argument"},
	{"setItem", module_setItem, METH_VARARGS, "Sets the history item specified by the argument"},
	{"newItem", module_newItem, METH_VARARGS, "Adds a new item at the top of the list"},
	{"clearHistory", module_clearHistory, METH_VARARGS, "Clears the whole history"},
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
	}
}

int get_plugin_info(char* module, plugin_info* info)
{
	init();
	PyObject* name = PyString_FromString(module);
	PyObject* m = PyImport_Import(name);
	Py_DECREF(name);
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
			Py_DECREF(infoFunc);
			Py_DECREF(m);
			return 1;
		}
		else
		{
			Py_DECREF(infoFunc);
			Py_DECREF(m);
		}
	}
	return 0;
}

void start_plugin(char* module)
{
	printf("plugin %s started\n", module);
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
