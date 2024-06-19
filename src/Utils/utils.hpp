#pragma once
#include <map>
#include <string>
#include "../Client/Client.hpp"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

#define log(msg) std::cout << YELLOW << msg << RESET << std::endl
#define Error(msg) std::cerr << RED << msg << RESET << std::endl

std::string getContentType(const std::string &filename);
std::string readFileToString(const std::string &filename);
std::string intToString(int value);
std::string generateDirectoryListing(const std::string &path);

enum HTTP_METHOD { GET, POST, PUT, DELETE, HEAD, OPTIONS, TRACE, CONNECT, PATCH, INVALID };
HTTP_METHOD find_method(const std::string &method);

bool isFolder(const std::string &path);
bool isFile(const std::string &path);
bool directory_contains_file(const std::string &directoryPath, std::string file_name);
void log_open_clients(std::map<int, Client *> &client_map);

size_t stringToSizeT(std::string str);
