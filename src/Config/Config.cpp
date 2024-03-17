#include "Config.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

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
	(void)filename;
	/* std::ifstream file(filename); */
	/* if (!file.is_open()) { */
	/* 	std::cerr << "Error: Could not open file " << filename << std::endl; */
	/* 	return; */
	/* } */

	/* std::string line; */
	/* while (std::getline(file, line)) { */
	/* 	std::istringstream iss(line); */
	/* 	std::string key; */
	/* 	if (std::getline(iss, key, '=')) { */
	/* 		std::string value; */
	/* 		if (std::getline(iss, value)) { */
	/* 			std::cout << "key: " << key << " value: " << value << std::endl; */
	/* 		} */
	/* 	} */
	/* } */

	/* file.close(); */
}
