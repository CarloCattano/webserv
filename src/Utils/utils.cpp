#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

std::map<std::string, std::string> content_types;

std::string getContentType(const std::string &filename)
{
	size_t dotPos = filename.find_last_of('.');
	if (dotPos != std::string::npos) {
		std::string extension = filename.substr(dotPos);
		if (content_types.find(extension) != content_types.end()) {
			return content_types[extension];
		}
	}
	return "text/plain";
}

std::string readFileToString(const std::string &filename)
{
	std::ifstream file(filename.c_str());

	if (file == 0) {
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::string intToString(int value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

std::string extract_requested_file_path(const char *buffer)
{
	std::string request(buffer);
	size_t start = request.find("GET") + 4;
	size_t end = request.find("HTTP/1.1") - 1;
	std::string path = request.substr(start, end - start);

	if (path == "/")
		return "/index.html";

	return path;
}
