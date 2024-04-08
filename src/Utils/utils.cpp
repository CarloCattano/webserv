#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
// getcwd include
#include <cerrno>
#include <unistd.h>

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

// get the path of the folder from where the server is run
std::string get_current_dir()
{
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		return std::string(cwd);
	else
		return "";
}

std::string extract_boundary(const char *buffer)
{
	std::string request(buffer);
	size_t start = request.find("boundary=") + 9;
	size_t end = request.find("\r\n") - 1;
	std::string boundary = request.substr(start, end - start);

	return boundary;
}

// extract request header
std::string extract_request_header(const char *buffer)
{
	std::string request(buffer);
	size_t end = request.find("\r\n\r\n") + 4;
	std::string header = request.substr(0, end);
	return header;
}

// extract request body
std::string extract_content_body(const char *buffer)
{
	std::string request(buffer);

	size_t start = request.find("\r\n\r\n") + 4;
	std::string body = request.substr(start);
	size_t start2 = body.find("\r\n\r\n") + 4;

	body = body.substr(start2);
	return body;
}
