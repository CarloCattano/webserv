#include "Config.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#define GREEN "\033[32m"
#define RESET "\033[0m"

Config::Config()
{
	this->port = 6969;
}

Config::~Config()
{
}

void Config::getPort(std::string line)
{
	std::string delimiter = " ";
	size_t pos = 0;
	std::string token;
	if (line.empty()) {
		std::cerr << "Error: empty line" << std::endl;
	}
	else {
		if ((pos = line.find("listen")) != std::string::npos) {
			line.erase(0, line.length() - 5);
			line.erase(line.length() - 1); // TODO : adjust for any port length
			std::stringstream ss(line);
			ss >> this->port;
		}
	}
}

Config::Config(const std::string filename)
{
	std::cout << "Reading config file: " << filename << std::endl;

	std::ifstream file(filename.c_str());
	std::string line;
	std::cout << "------------------" << std::endl;
	while (std::getline(file, line)) {
		getPort(line);
		std::cout << GREEN << line << "\n" << RESET;
	}
	std::cout << "------------------" << std::endl;
}
