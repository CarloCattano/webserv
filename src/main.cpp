#include <iostream>
#include <map>
#include <string>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "./Utils/utils.hpp"
#include "./Config/Config.hpp"
#include "./Server/Server.hpp"

void populateContentTypes()
{
	content_types[".html"] = "text/html";
	content_types[".php"] = "text/html";
	content_types[".css"] = "text/css";
	content_types[".jpg"] = "image/jpeg";
	content_types[".jpeg"] = "image/jpeg";
	content_types[".png"] = "image/png";
	content_types[".gif"] = "image/gif";
	content_types[".py"] = "text/html";
}

int main(int argc, char *argv[])
{
	populateContentTypes();

	std::string config_file = "";

	if (argv[1] == NULL || argc != 2) {
		// use default config file as per the project requirement
		config_file = "server.conf";
		std::cout << "Using default configuration file: " << config_file << std::endl;
	}
	else {
		config_file = argv[1];

		if (config_file == "" || config_file.empty()) {
			std::cerr << "Invalid configuration file" << std::endl;
			return 1;
		}
	}

	Config config(config_file);
	Server server(config.get_virtual_servers()[0]);

	return 0;
}
