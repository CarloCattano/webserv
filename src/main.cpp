#include <iostream>
#include "./Server/Server.hpp"


void populateContentTypes()
{
	content_types[".html"] = "text/html";
	content_types[".css"] = "text/css";
	content_types[".jpg"] = "image/jpeg";
	content_types[".jpeg"] = "image/jpeg";
	content_types[".png"] = "image/png";
	// Add more file extensions and corresponding content types as needed
}

int main(int argc, char *argv[])
{
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
