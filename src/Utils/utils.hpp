#pragma once
#include <map>
#include <string>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

#define log(msg) std::cout << YELLOW << msg << RESET << std::endl
#define Error(msg) std::cerr << RED << msg << RESET << std::endl

extern std::map<std::string, std::string> content_types;

std::string getContentType(const std::string &filename);
std::string readFileToString(const std::string &filename);
std::string intToString(int value);
std::string extract_requested_file_path(const char *buffer);
