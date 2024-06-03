#include "utils.hpp"
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

std::map<std::string, std::string> content_types;

HttpMethod get_http_method(const char *request) {
	// Find the first space in the request line
	const char *first_space = strchr(request, ' ');
	if (first_space != NULL) {
		// Extract the HTTP method from the request line
		std::string method(request, first_space - request);
		if (method == "GET") {
			return GET;
		} else if (method == "POST") {
			return POST;
		} else if (method == "DELETE") {
			return DELETE;
		}
	}
	return UNKNOWN; // Unable to determine HTTP method
}

std::map<std::string, std::string> getAllContentTypes() {
	std::map<std::string, std::string> content_types;

	content_types[".html"] = "text/html";
	content_types[".php"] = "text/html";
	content_types[".css"] = "text/css";
	content_types[".jpg"] = "image/jpeg";
	content_types[".jpeg"] = "image/jpeg";
	content_types[".png"] = "image/png";
	content_types[".gif"] = "image/gif";
	content_types[".py"] = "text/html";

	return (content_types);
}

std::string getContentType(const std::string &filename) {
	std::map<std::string, std::string> content_types;
	content_types = getAllContentTypes();

	size_t dotPos = filename.find_last_of('.');
	if (dotPos != std::string::npos) {
		std::string extension = filename.substr(dotPos);
		if (content_types.find(extension) != content_types.end()) {
			return content_types[extension];
		}
	}
	return "text/plain";
}

std::string readFileToString(const std::string &filename) {
	std::ifstream file(filename.c_str());

	if (!file) {
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::string intToString(int value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

std::string extract_requested_file_path(const char *buffer) {
	std::string request(buffer);
	size_t start = 0;

	if (request.find("GET") == 0) {
		start = request.find("GET") + 4;
	} else if (request.find("POST") == 0) {
		start = request.find("POST") + 5;
	} else if (request.find("DELETE") == 0) {
		start = request.find("DELETE") + 7;
	}

	size_t end = request.find("HTTP/1.1") - 1;
	std::string path = request.substr(start, end - start);

	return path;
}

std::string extract_filename_from_request(const char *request) {
	const char *filename_field = strstr(request, "filename=");
	if (filename_field != NULL) {
		const char *filename_start = filename_field + strlen("filename=");
		const char *filename_end = strstr(filename_start, "\r\n");
		if (filename_end != NULL) {
			std::string filename(filename_start, filename_end - filename_start);
			return filename;
		}
	}
	return "";
}

bool is_file_upload_request(const char *request) {
	const char *content_type_header = strstr(request, "Content-Type:");
	if (content_type_header != NULL) {
		const char *multipart_form_data = strstr(content_type_header, "multipart/form-data");
		if (multipart_form_data != NULL) {
			return true;
		}
	}
	return false;
}

// get the path of the folder from where the server is run
std::string get_current_dir() {
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		return std::string(cwd);
	else
		return "";
}

std::string generateDirectoryListing(const std::string &path) {
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
		} else {
			if (entryName.find(".html") == std::string::npos) {
				html << "<li><a href=\"" << entryName << "\" download>" << entryName << "</a></li>";
			} else {
				html << "<li><a href=\"" << entryName << "\">" << entryName << "</a></li>";
			}
		}
	}

	// Close the directory
	closedir(dir);

	html << "</ul></body></html>";
	return html.str();
}

bool isFolder(const std::string &path) {
	struct stat buffer;
	if (stat(path.c_str(), &buffer) != 0) // file does not exist
		return false;
	if (S_ISDIR(buffer.st_mode)) // file is a directory
		return true;
	return false;
}

bool isFile(const std::string &path) {
	struct stat buffer;
	if (stat(path.c_str(), &buffer) != 0) // file does not exist
		return false;
	if (S_ISREG(buffer.st_mode)) // file is a regular file
		return true;
	return false;
}
