#include <string>

class FileUploader {
  public:
	FileUploader();
	~FileUploader();

	void handle_file_upload(int client_fd, const std::string &filename, int file_size);
};
