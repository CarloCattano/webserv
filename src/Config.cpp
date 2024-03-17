#include "Config.hpp"
#include <fstream>
#include <iostream>

Config::Config(const std::string &filename) : filename(filename)
{
	if (!getConfig())
		throw std::runtime_error("Error: Unable to read config file " + filename);
}

Config::~Config()
{
}

std::string Config::get(const std::string &section, const std::string &key) const
{
	std::map<std::string, std::map<std::string, std::string> >::const_iterator it =
		settings.find(section);

	if (it != settings.end()) {
		std::map<std::string, std::string>::const_iterator value_it = it->second.find(key);
		std::map<std::string, std::string> section_map = it->second;
		if (value_it != section_map.end())
			return value_it->second;
	}
	return "";
}

std::string trimInput(const std::string &str)
{
	size_t start = str.find_first_not_of(" \t");
	size_t end = str.find_last_not_of(" \t");
	if (start == std::string::npos || end == std::string::npos)
		return "";
	return str.substr(start, end - start + 1);
}

bool Config::getConfig()
{
	std::ifstream file(filename.c_str());

	if (!file.is_open()) {
		std::cerr << "Error: Unable to open config file " << filename << std::endl;
		return false;
	}

	std::string line;
	std::string section;
	while (std::getline(file, line)) {
		line = trimInput(line);
		if (line.empty() || line[0] == '#')
			continue;

		if (line[0] == '[' && line[line.size() - 1] == ']') {
			size_t pos = line.find('=');
			if (pos != std::string::npos) {
				std::string key = trimInput(line.substr(0, pos));
				std::string value = trimInput(line.substr(pos + 1));
				settings[section][key] = value;
			}
		}
	}
	return true;
}
