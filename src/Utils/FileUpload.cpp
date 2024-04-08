#include "FileUpload.hpp"
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

FileUploader::FileUploader()
{
}

FileUploader::~FileUploader()
{
}

void FileUploader::handle_file_upload(int client_fd,
									  const std::string &filename,
									  int file_size,
									  const char *start)
{
	char buffer[BUFFER_SIZE];

	ssize_t total_bytes_received = 0; // Total bytes received from the client
	ssize_t bytes_received;

	std::string cleanFile;

	if (filename[0] == '"' && filename[filename.size() - 1] == '"') {
		cleanFile = filename.substr(1, filename.size() - 2);
	}
	else {
		cleanFile = filename;
	}

	int outfile = open(cleanFile.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (outfile == -1) {
		perror("open");
		return;
	}

	total_bytes_received += write(outfile, start, strlen(start));

	while (total_bytes_received < file_size) {
		bytes_received = recv(client_fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);
		if (bytes_received > 0) {
			ssize_t bytes_written = write(outfile, buffer, bytes_received);
			if (bytes_written == -1) {
				perror("write");
				close(outfile);
				return;
			}
			total_bytes_received += bytes_written;
		}
		else if (bytes_received == 0) {
			break;
		}
		else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				usleep(1000); // Sleep for 1 millisecond
				continue;
			}
			else {
				perror("recv");
				break;
			}
		}
	}

	// add a null terminator to the file
	write(outfile, "\0", 1);
	close(outfile);

	/* if (total_bytes_received == file_size) { */
	/* 	std::cout << "File '" << filename << "' uploaded successfully." << std::endl; */
	/* } */
	/* else { */
	/* 	std::cerr << "Incomplete upload for file '" << filename << "'." << std::endl; */
	/* } */
}
