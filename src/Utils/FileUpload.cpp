#include "FileUpload.hpp"
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

FileUploader::FileUploader() {}

FileUploader::~FileUploader() {}

void FileUploader::handle_file_upload(int client_fd, const std::string &filename, int file_size) {

	const int BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];

	ssize_t total_bytes_received = 0; // Total bytes received from the client
	ssize_t bytes_received;

	// Open file for writing
	int outfile = open(filename.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (outfile == -1) {
		perror("open");
		return;
	}

	std::cout << "Receiving file '" << filename << "' of size " << file_size << " bytes."
			  << std::endl;
	std::cout << "Writing to file '" << filename << "'." << std::endl;

	while (total_bytes_received < file_size) {
		bytes_received = recv(client_fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);
		if (bytes_received > 0) {
			if (total_bytes_received + bytes_received > file_size - 6) {
				break;
			}
			ssize_t bytes_written = write(outfile, buffer, bytes_received);
			if (bytes_written == -1) {
				perror("write");
				close(outfile);
				return;
			}
			total_bytes_received += bytes_written;
		} else if (bytes_received == 0) {
			// Client closed connection
			break;
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// No more data available for now, wait and try again
				usleep(1000); // Sleep for 1 millisecond
				continue;
			} else {
				// Other error
				perror("recv");
				break;
			}
		}
		std::cout << "Received " << total_bytes_received << " bytes." << std::endl;
	}

	// Close the file
	close(outfile);
	std::cout << "File closed" << std::endl;
	// Check if all data has been received
	if (total_bytes_received == file_size) {
		std::cout << "File '" << filename << "' uploaded successfully." << std::endl;
	} else {
		std::cerr << "Incomplete upload for file '" << filename << "'." << std::endl;
	}
}
