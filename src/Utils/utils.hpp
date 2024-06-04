#pragma once
#include <map>
#include <string>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

#define log(msg) std::cout << YELLOW << msg << RESET << std::endl
#define Error(msg) std::cerr << RED << msg << RESET << std::endl

std::string getContentType(const std::string &filename);
std::string readFileToString(const std::string &filename);
std::string intToString(int value);
std::string extract_requested_file_path(const char *buffer);
std::string get_current_dir();
std::string generateDirectoryListing(const std::string &path);

bool is_file_upload_request(const char *request);
std::string extract_filename_from_request(const char *request);
std::size_t extract_content_length(const char *request);

void populateContentTypes(std::map<std::string, std::string> &content_types);

enum HttpMethod { GET, POST, DELETE, UNKNOWN };
HttpMethod get_http_method(const char *buffer);

bool isFolder(const std::string &path);
bool isFile(const std::string &path);
bool directory_contains_index_file(const std::string& directoryPath);