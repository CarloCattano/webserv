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
	content_types[".css"] = "text/css";
	content_types[".jpg"] = "image/jpeg";
	content_types[".jpeg"] = "image/jpeg";
	content_types[".png"] = "image/png";
	content_types[".py"] = "text/html";
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " [configuration file]" << std::endl;
		return 1;
	}
	populateContentTypes();

	Config config(argv[1]); // TODO

	Server server("1234", 8080);
	server.start();

	return 0;
}
