
#include <iostream>
#include "./Server/Server.hpp"

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

const int MAX_EVENTS = 10;
const int BACKLOG = 10;
const int BUFFER_SIZE = 1024;

int PORT = 8080;

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
