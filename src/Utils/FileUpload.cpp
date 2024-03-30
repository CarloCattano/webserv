#include "FileUpload.hpp"
#include <iostream>
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

FileUploader::FileUploader()
{
}

FileUploader::~FileUploader()
{
}

void FileUploader::handle_file_upload(int client_fd, const std::string &filename, int file_size)
{
	const int BUFFER_SIZE = 1024; // Choose an appropriate buffer size
	char buffer[BUFFER_SIZE];
	ssize_t total_bytes_received = 0; // Total bytes received from the client
	ssize_t bytes_received;
	int file_fd;

	bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
	if (bytes_received <= 0) {
		perror("Error receiving filename");
		return;
	}

	buffer[bytes_received] = '\0'; // Null-terminate the received filename

	file_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (file_fd == -1) {
		perror("Error opening file");
		return;
	}

	// Read data from client and write to file
	while (total_bytes_received < file_size) {
		bytes_received = recv(client_fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);
		if (bytes_received > 0) {
			std::cout << "Bytes received: " << bytes_received << std::endl;
			ssize_t bytes_written = write(file_fd, buffer, bytes_received);
			if (bytes_written < 0) {
				perror("Error writing to file");
				break;
			}
			std::cout << "Bytes written: " << bytes_written << std::endl;
			total_bytes_received += bytes_received; // Update total bytes received
		}
		else if (bytes_received == 0) {
			std::cout << "Client closed connection" << std::endl;
			break;
		}
		else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// No data available, wait and try again
				/* usleep(1000); // Sleep for 1 millisecond */
				/* std::cout << "No data available, waiting..." << std::endl; */
				continue;
			}
			else {
				// Other error
				perror("Error receiving data from client");
				break;
			}
		}
	}

	close(file_fd);
	std::cout << "File closed" << std::endl;
}
