#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
// getcwd include
#include <cerrno>
#include <dirent.h>
#include <sys/stat.h>
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

	/* if (path == "/") */
	/* return "/index.html"; */

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
	size_t start = request.find("\r\n\r\n") + 4; // +4 to skip the \r\n\r\n
	std::string body = request.substr(start);

	size_t boundary_pos = request.find("boundary=");
	if (boundary_pos == std::string::npos) {
		return "";
	}

	// store the boundary string
	std::string boundary = extract_boundary(buffer);

	size_t start2 = body.find("\r\n\r\n") + 4;

	body = body.substr(start2);

	size_t end = body.find("\r\n-");
	if (end != std::string::npos) {
		body = body.substr(0, end);
	}

	/* 	size_t last_carriage_return = body.rfind("\r"); */
	/* 	if (last_carriage_return != std::string::npos) { */
	/* 		body.erase(last_carriage_return); */
	/* 	} */

	/* 	for (size_t pos = body.find(boundary); pos != std::string::npos; */
	/* 		 pos = body.find(boundary, pos + 1)) { */
	/* 		body.erase(pos, boundary.length()); */
	/* 	} */

	return body;
}

std::string generateDirectoryListing(const std::string &path)
{
	std::stringstream html;
	html << "<html><head><title>Directory Listing</title></head><body><h1>Directory "
			"Listing</h1><ul>";

	// Add "../" entry
	html << "<li><a href=\"../\">../</a></li>";

	// Open the directory
	DIR *dir = opendir(path.c_str());
	if (dir == NULL) {
		html << "<p>Error opening directory " << path << "</p>";
		html << "</ul></body></html>";
		return html.str();
	}

	// Read directory entries
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string entryName = entry->d_name;
		// Exclude the current directory "." and any other directories starting with "."
		if (entryName == "." || entryName[0] == '.')
			continue;
		std::string fullPath = path + "/" + entryName;
		struct stat entry_stat;
		if (stat(fullPath.c_str(), &entry_stat) == 0 && S_ISDIR(entry_stat.st_mode)) {
			// It's a directory, create a hyperlink to navigate into the folder
			html << "<li><a href=\"" << entryName << "/\">" << entryName << "/</a></li>";
		}
		else {
			// It's a file, create a link to download the file
			html << "<li><a href=\"" << entryName << "\" download>" << entryName << "</a></li>";
		}
	}

	// Close the directory
	closedir(dir);

	html << "</ul></body></html>";
	return html.str();
}
