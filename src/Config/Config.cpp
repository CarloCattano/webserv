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

// void getPort(std::string line)
// {
// 	std::string delimiter = " ";
// 	size_t pos = 0;
// 	std::string token;
// 	if (line.empty()) {
// 		std::cerr << "Error: empty line" << std::endl;
// 	}
// 	else {
// 		if ((pos = line.find("listen")) != std::string::npos) {
// 			line.erase(0, line.length() - 5);
// 			line.erase(line.length() - 1); // TODO : adjust for any port length
// 			std::stringstream ss(line);
// 			ss >> this->port;
// 		}
// 	}
// }

std::string get_file_content(const std::string &filename) {
	std::ifstream file(filename.c_str());
	std::string line;
	std::string file_content;

    if (file.is_open()) { 
        while (std::getline(file, line)) {
            file_content += line + '\n';
        }
        file.close();
    } else {
        std::cerr << "Failed to open the file." << std::endl;
		// throw error????
    }
	file_content += '\0'; 
	return (file_content);
}

bool is_whitespace_char(char c) {
	if (c == ' ' || c == '\t' || c == '\n')
		return (true);
	else
		return (false);	
}

Virtual_Server_Config read_virtual_server_obj(std::string str, int i) {
	Virtual_Server_Config virtual_server;

	while (is_whitespace_char(str[i])) {i++;}
	if (str[i++] != '{') {
		throw std::runtime_error("Starting curly brace missing: '{'");
	}
	while (str[i] && str[i] != '}') {
		std::vector<std::string> values;
		while (is_whitespace_char(str[i])) {i++;}
		while (str[i] && str[i] != ';' && str[i] != '\n' && str[i] != '}') {
			std::string single_value;
			while (str[i] == ' ') {i++;}
			while (str[i] && str[i] != ';' && str[i] != '\n' && str[i] != '}' && str[i] != ' ') {
				single_value+= str[i++];
			}
			values.push_back(single_value);
			i++;
			std::cout << single_value << " ";
		}
		if (str[i] && str[i] != '}')
			i++;
		std::cout << std::endl;
		if (values[0] == "listen") {
			virtual_server.port = atoi(values[1].c_str());
		}
	}
	if (str[i] != '}') {
		throw std::runtime_error("Ending curly brace missing: '}'");
	}
	std::cout << std::endl;
	return (virtual_server);
}

Config::Config(const std::string filename)
{
	Virtual_Server_Config virtual_server_obj;
	std::string file_content = get_file_content(filename);

	size_t index = file_content.find("server");
	while (index != std::string::npos) {
		virtual_server_obj = read_virtual_server_obj(file_content, index + 6);
		this->virtual_servers.push_back(virtual_server_obj);
		while (file_content[index] && file_content[index] != '}') {index++;}
		index = file_content.find("server", index);
	}
}

std::vector<Virtual_Server_Config> Config::get_virtual_servers()
{

	return (this->virtual_servers);
}