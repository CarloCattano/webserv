#include "Cgi.hpp"
#include <cstdlib>
#include <iostream>
// imports for cgi execution
#include "utils.hpp"
#include <stdexcept>
#include <stdio.h>

Cgi::Cgi() {}

Cgi::~Cgi() {}

Cgi &Cgi::operator=(const Cgi &src) {
	if (this != &src) {
		this->_cgi = src._cgi;
	}
	return *this;
}

Cgi::Cgi(const Cgi &src) { *this = src; }

void runCommand(const std::string &command, std::string &result) {
	FILE *pipe = popen(command.c_str(), "r");
	if (!pipe) {
		std::cerr << "popen failed" << std::endl;
		exit(1);
	}
	char buffer[128];
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	pclose(pipe);
}

std::string Cgi::run() const {

	std::string path = "$HOME/webserv/website/cgi-bin/hello.py";

	if (path.empty())
		throw std::runtime_error("Path to python script is empty");

	std::string command = "/usr/bin/python3 " + path;
	std::string content_type = "text/html";
	std::string result = "";

	runCommand(command, result);

	result = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type + "\r\n" +
			 "Content-Length: " + intToString(result.length()) + "\r\n\r\n" + result + "\r\n\r\n";

	return result;
}
