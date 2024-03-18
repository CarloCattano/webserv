#include <iostream>
#include <map>
#include <string>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Config.hpp"
#include "Server.hpp"
#include "utils.hpp"

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
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " [configuration file]" << std::endl;
		return 1;
	}
	populateContentTypes();

	try {
		Config config(argv[1]);
		Server server("1234", config.get_virtual_servers()[0].port);
		server.start();
	}
	catch (std::exception &e) {
		Error("ERROR : " << e.what());
		return 1;
	}

	return 0;
}
