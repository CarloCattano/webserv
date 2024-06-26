#include "Config.hpp"
#define GREEN "\033[32m"
#define RESET "\033[0m"

#include <fstream>
#include <iostream>

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
		throw std::runtime_error("Unable to open file: " + filename);
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

template <typename T>
void print_vector(T vector) {
	for (size_t i = 0; i < vector.size(); ++i) {
		std::cout << vector[i] << " ";
	}
	std::cout << std::endl;
}

void print_server_routes(std::vector<Route> routes) {
	for (size_t i = 0; i < routes.size(); ++i) {
		std::cout << "\tRoute No." << i + 1 << std::endl;
		if (routes[i].location != "")
			std::cout << "\t\tLocation: " << routes[i].location << std::endl;
		if (routes[i].matching_style != "")
			std::cout << "\t\tMatching: " << routes[i].matching_style << std::endl;
		if (routes[i].root != "")
			std::cout << "\t\tRoot: " << routes[i].root << std::endl;
		if (routes[i].redirection.code)
			std::cout << "\t\tRedirection: " << routes[i].redirection.code << " " << routes[i].redirection.url
					  << std::endl;
		std::cout << "\t\tAutoIndex: " << routes[i].autoindex << std::endl;
		if (routes[i].index_file != "")
			std::cout << "\t\tIndex File: " << routes[i].index_file << std::endl;
		std::cout << "\t\tPOST: " << routes[i].POST.is_allowed << std::endl;
		std::cout << "\t\tGET: " << routes[i].GET.is_allowed << std::endl;
		std::cout << "\t\tDELETE: " << routes[i].DELETE.is_allowed << std::endl;
		if (routes[i].cgi_path != "")
			std::cout << "\t\tcgi_path: " << routes[i].cgi_path << std::endl;
		if (routes[i].cgi_extension != "")
			std::cout << "\t\tcgi_extension: " << routes[i].cgi_extension << std::endl;
	}
	std::cout << std::endl;
}

void print_server_obj(Server &obj) {
	std::cout << GREEN << "SERVER " << RESET << std::endl;
	if (obj.getPort())
		std::cout << "\tPort: " << obj.getPort() << std::endl;
	if (obj.getServerNames().size() > 0) {
		std::cout << "\tServer names: ";
		print_vector(obj.getServerNames());
	}
	if (obj.getErrorPages().size() > 0) {
		std::cout << "\tError pages: ";
		print_vector(obj.getErrorPages());
	}
	std::cout << "\tRoot " << obj.getRoot(NULL) << std::endl;
	std::cout << "\tIndexFile: " << obj.getIndexFile(NULL) << std::endl;
	std::cout << "\tAutoIndex: " << obj.getAutoindex(NULL) << std::endl;
	std::cout << "\tClientMaxBodySize " << obj.getClientMaxBodySize() << std::endl;
	std::cout << "\tPOST: " << obj.getPost(NULL).is_allowed << std::endl;
	std::cout << "\tGET: " << obj.getGet(NULL).is_allowed << std::endl;
	std::cout << "\tDELETE: " << obj.getDelete(NULL).is_allowed << std::endl;
	std::cout << "\tcgi_path: " << obj.getCgiPath() << std::endl;
	std::cout << "\tcgi_extension: " << obj.getCgiExtension() << std::endl;
	if (obj.getRedirection(NULL).code)
		std::cout << "\tRedirection: " << obj.getRedirection(NULL).code << " " << obj.getRedirection(NULL).url
				  << std::endl;
	if (obj.getRoutes().size() > 0)
		print_server_routes(obj.getRoutes());
	std::cout << std::endl;
}

std::string toLowerCase(std::string str) {
	std::string lowerCaseStr;
	long unsigned int i = 0;
	while (i < str.size()) {
		lowerCaseStr += std::tolower(str[i]);
		i++;
	}
	return (lowerCaseStr);
}

int iterate_to_next_server_line(std::string str, int i) {
	bool is_comment = false;

	if (str[i] == '#')
		is_comment = true;
	while (str[i] && str[i] != '\n') {
		if (!is_comment && (str[i] == '}' || str[i] == ';'))
			break;
		i++;
	}
	while (str[i] && str[i] != '}' && (str[i] == ';' || is_whitespace_char(str[i])))
		i++;
	return (i);
}

std::vector<std::string> convert_server_line_2_vector(std::string str, int i) {
	std::vector<std::string> values;

	while (str[i] && str[i] != '}' && str[i] != ';' && str[i] != '\n') {
		std::string single_value;
		while (str[i] && str[i] != '}' && str[i] != ';' && str[i] != '\n' && str[i] != ' ') {
			single_value += str[i++];
		}
		values.push_back(single_value);
		while (str[i] == ' ') {
			i++;
		}
	}
	return (values);
}

int iterate_to_first_server_line(std::string str, int i) {
	while (is_whitespace_char(str[i])) {
		i++;
	}
	if (str[i++] != '{')
		throw std::runtime_error("Starting curly brace missing: '{'");
	while (is_whitespace_char(str[i])) {
		i++;
	}
	return (i);
}

bool isNumeric(const std::string &str) {
	// Check if the string is empty
	if (str.empty()) {
		return false;
	}

	// Check each character
	for (size_t i = 0; i < str.size(); ++i) {
		if (!isdigit(str[i])) {
			return false;
		}
	}

	return true;
}
