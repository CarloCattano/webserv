#pragma once
#include <string>

enum ALOWED {
	YES = 0,
	NO = -1,
	FOLDER = -2,
	EXISTS = 2,
};

#include <vector>

struct MultipartFormData {
	std::string fileName;
	std::vector<char> fileContent;
};


class FileUploader {
public:
	FileUploader();
	~FileUploader();

	MultipartFormData parse_multipart_form_data(const std::string &boundary, const std::string &body);

	void handle_file_upload(int client_fd, const std::string &filename, int file_size, const char *start);
};
