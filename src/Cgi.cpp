#include "Cgi.hpp"
#include <iostream>

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

// run function
void Cgi::run(const std::string &path, const std::string &filename)
{
	if (filename.find(".py") != std::string::npos) {
		this->_runPython(path, filename, "");
	}
}

void Cgi::_runPython(const std::string &path, const std::string &filename, const std::string &query)
{
	std::string command = "python3 " + path + "/" + filename + " " + query;
	std::cout << "command: " << command << std::endl;
}
