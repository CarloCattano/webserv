#include "Config.hpp"

#include <fstream>
#include <iostream>

Config::Config()
{
}

Config::~Config()
{
}

// read the config and print the key value pairs
// for now as a start
Config::Config(const std::string filename)
{
	std::cout << "Reading config file: " << filename << std::endl;

	std::ifstream file(filename.c_str());
	std::string line;
	std::cout << "------------------" << std::endl;
	while (std::getline(file, line)) {
		std::cout << line << std::endl;
	}
	std::cout << "------------------" << std::endl;
}
