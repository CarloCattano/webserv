#pragma once
#include <map>
#include <string>

extern std::map<std::string, std::string> content_types;

std::string getContentType(const std::string &filename);
std::string readFileToString(const std::string &filename);
std::string intToString(int value);
std::string extract_requested_file_path(const char *buffer);
