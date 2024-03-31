#include "FileUpload.hpp"
#include <fstream>
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
	const int BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];
	ssize_t total_bytes_received = 0; // Total bytes received from the client
	ssize_t bytes_received;

	std::ofstream outfile(filename.c_str(), std::ios::binary);

	if (!outfile) {
		std::cerr << "Failed to open file for writing." << std::endl;
		return;
	}

	std::cout << "File opened" << std::endl;
	std::cout << "File size: " << file_size << std::endl;
	std::cout << "Filename: " << filename << std::endl;
	std::cout << "Client fd: " << client_fd << std::endl;
	std::cout << "Buffer size: " << BUFFER_SIZE << std::endl;
	std::cout << "Total bytes received: " << total_bytes_received << std::endl;

	while (total_bytes_received < file_size) {
		bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
		if (bytes_received > 0) {
			outfile.write(buffer, bytes_received);
			total_bytes_received += bytes_received; // Update total bytes received
		}
		else if (bytes_received == 0) {
			std::cout << "Client closed connection" << std::endl;
			break;
		}
		else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// No data available, wait and try again
				usleep(100); // Sleep for 1 millisecond
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

	std::cout << "Total bytes received: " << total_bytes_received << std::endl;
	std::cout << "File size: " << file_size << std::endl;

	outfile.close();
	std::cout << "File closed" << std::endl;
}
