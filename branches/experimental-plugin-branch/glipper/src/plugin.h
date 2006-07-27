
struct plugin_info {
	char* name;
	char* descr;
	//Maybe options??
};

//Common functions for the plugin system:

void init_plugin_system();

struct plugin_info get_plugin_info(char* file);

void start_plugin(char* file);

void stop_plugin(char* file);

//Sending Events to the plugins:
//
//TODO
