
struct plugin_info {
	char* name;
	char* descr;
	//Maybe options??
};

//Common functions for the plugin system:

struct plugin_info get_plugin_info(char* module);

void start_plugin(char* module);

void stop_plugin(char* module);

//Sending Events to the plugins:
//
//TODO
