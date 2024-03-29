#include "FileUpload.hpp"
#include <iostream>
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include "utils.hpp"

FileUploader::FileUploader()
{
}

FileUploader::~FileUploader()
{
}

void FileUploader::handle_file_upload(int client_fd, const std::string &filename, int file_size)
{
	int BUFFER_SIZE = file_size;
	int MAX_FILENAME_LEN = 256;
	char buffer[BUFFER_SIZE];
	ssize_t bytes_received;
	int file_fd;

	bytes_received = recv(client_fd, buffer, MAX_FILENAME_LEN, 0);
	if (bytes_received <= 0) {
		perror("Error receiving filename");
		return;
	}

	std::cout << "Received filename: " << buffer << std::endl;
	std::cout << "Bytes received: " << bytes_received << std::endl;
	// Open a new file for writing to it
	file_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);

	// Read data from the client and write to the file
	while ((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
		ssize_t bytes_written = write(file_fd, buffer, bytes_received);
		if (bytes_written < 0) {
			perror("Error writing to file");
			break;
		}
	}

	// Check for errors or end-of-file
	if (bytes_received < 0) {
		perror("Error receiving data from client");
	}

	// Close the file
	close(file_fd);
}
