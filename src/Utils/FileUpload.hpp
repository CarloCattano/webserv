#include <string>

class FileUploader {
public:
	FileUploader();
	~FileUploader();

	void handle_file_upload(int client_fd, const std::string &filename, int file_size);

private:
	FILE *file;
	int total_bytes_written;
	int bytes_received;
};
