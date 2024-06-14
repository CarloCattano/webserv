#include "utils.hpp"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

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


/** @brief
 *  This method is responsible for generating the directory listing. Files can be Downloaded and Directories
 *  can be navigated. It generates the HTML format of the directory listing.
 *  Uses the opendir() and readdir() functions to read the contents of the directory and generate the listing. The
 *  *  @param path: path of the directory
 *  @return std::string: directory listing in HTML format
 */

std::string generateDirectoryListing(const std::string &path) {
	std::stringstream html;
	html << "<html><head><title>Directory Listing</title></head><body><h1>Directory "
			"Listing</h1><ul>";

	// Add "../" entry
	html << "<li><a href=\"../\">../</a></li>";

	DIR *dir = opendir(path.c_str());
	if (dir == NULL) {
		html << "<p>Error opening directory " << path << "</p>";
		html << "</ul></body></html>";
		return html.str();
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string entryName = entry->d_name;
		if (entryName == "." || entryName[0] == '.')
			continue;
		std::string fullPath = path + "/" + entryName;
		struct stat entry_stat;
		if (stat(fullPath.c_str(), &entry_stat) == 0 && S_ISDIR(entry_stat.st_mode)) {
			html << "<li><a href=\"" << entryName << "/\">" << entryName << "/</a></li>";
		} else {												// It's a file
			if (entryName.find(".html") == std::string::npos) { // If it's not an HTML file, add a download link
				html << "<li><a href=\"" << entryName << "\" download>" << entryName << "</a></li>";
			} else {
				html << "<li><a href=\"" << entryName << "\">" << entryName << "</a></li>";
			}
		}
	}

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
	if (isFolder(path))
		return false;
	return true;
}

void log_open_clients(std::map<int, Client> &clients) {
	std::cout << YELLOW << "Open clients: " << clients.size() << RESET << std::endl;
	std::cout << "Client fds: ";
	for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); it++) {
		std::cout << GREEN << it->first << " " << it->second.getRequest().request << " " << RESET;
	}
	std::cout << RESET << std::endl;
}

bool directory_contains_file(const std::string &directoryPath, std::string file_name) {
	DIR *dir = opendir(directoryPath.c_str());
	if (!dir) {
		std::cerr << "Error opening directory: " << directoryPath << std::endl;
		return false;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN) {
			if (strcmp(file_name.c_str(), entry->d_name) == 0) {
				closedir(dir);
				return true;
			}
		}
	}

	closedir(dir);
	return false;
}

HTTP_METHOD find_method(const std::string &method) {
	if (method == "GET")
		return GET;
	if (method == "POST")
		return POST;
	if (method == "DELETE")
		return DELETE;
	if (method == "PUT")
		return PUT;
	if (method == "HEAD")
		return HEAD;
	if (method == "OPTIONS")
		return OPTIONS;
	if (method == "TRACE")
		return TRACE;
	if (method == "CONNECT")
		return CONNECT;
	if (method == "PATCH")
		return PATCH;
	return INVALID;
}
