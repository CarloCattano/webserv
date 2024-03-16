
#include "./Server/Server.hpp"
#include <iostream>

#include "Cgi.hpp"
#include "utils.hpp"
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

void populateContentTypes() {
	content_types[".html"] = "text/html";
	content_types[".css"] = "text/css";
	content_types[".jpg"] = "image/jpeg";
	content_types[".jpeg"] = "image/jpeg";
	content_types[".png"] = "image/png";
	content_types[".py"] = "text/html";
}

int main(int argc, char *argv[]) {

	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " [configuration file]" << std::endl;
		return 1;
	}
	populateContentTypes();

	// TODO Parse configuration here

	Server server("1234", 8080);
	server.start();

	return 0;
}
