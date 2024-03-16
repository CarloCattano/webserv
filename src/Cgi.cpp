#include "Cgi.hpp"
#include <cstdlib>
#include <iostream>
// imports for cgi execution
#include <stdio.h>
#include "utils.hpp"

Cgi::Cgi()
{
}

Cgi::~Cgi()
{
}

Cgi &Cgi::operator=(const Cgi &src)
{
	if (this != &src) {
		this->_cgi = src._cgi;
	}
	return *this;
}

Cgi::Cgi(const Cgi &src)
{
	*this = src;
}

// run a command on the system and return the return in a std::string

void runCommand(const std::string &command, std::string &result)
{
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

std::string Cgi::run() const
{
	std::string path = "/home/carlo/42/webserv/website/cgi-bin";
	std::string command = "/sbin/python3 " + path + "/hello.py";
	std::string content_type = "text/html";
	std::string result = "";
	std::string file_path = readFileToString(path + "/hello.py");

	runCommand(command, result);
	result = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type + "\r\n" +
		"Content-Length: " + intToString(file_path.length()) + "\r\n\r\n" + result + "\r\n\r\n";

	return result;
}
