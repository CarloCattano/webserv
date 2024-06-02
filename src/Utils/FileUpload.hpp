#include <string>

enum ALOWED {
    YES = 0,
    NO = -1,
    FOLDER = -2,
    EXISTS = 2,
};

class FileUploader {
  public:
	FileUploader();
	~FileUploader();

	void handle_file_upload(int client_fd, const std::string &filename, int file_size,
							const char *start);
};
