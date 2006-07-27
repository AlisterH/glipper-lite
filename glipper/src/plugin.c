#include <Python.h>
#include "plugin.h"

void init_plugin_system()
{
	Py_Initialize();
}

struct plugin_info get_plugin_info(char* file)
{
}

void start_plugin(char* file)
{
}

void stop_plugin(char* file)
{
}

