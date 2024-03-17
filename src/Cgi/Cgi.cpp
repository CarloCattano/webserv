#include "Cgi.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>
#include "utils.hpp"

Cgi::Cgi()
{
}

Cgi::~Cgi()
{
}

Cgi &Cgi::operator=(const Cgi &src)
{
	if (this != &src) {
		this->_cgi = src._cgi;
	}
	return *this;
}

Cgi::Cgi(const Cgi &src)
{
	*this = src;
}

std::string runCommand()
{
	std::string path = "website/cgi-bin/hello.py";

	if (path.empty())
		throw std::runtime_error("Path to python script is empty");

	// Create pipes for communication between parent and child processes
	int pipe_fd[2];
	if (pipe(pipe_fd) == -1) {
		std::cerr << "Failed to create pipe" << std::endl;
		exit(EXIT_FAILURE);
	}

	pid_t pid = fork();
	if (pid == -1) {
		std::cerr << "Failed to fork process" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (pid == 0) { // Child process
		// Close the read end of the pipe
		close(pipe_fd[0]);

		// Redirect stdout to the write end of the pipe
		if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
			std::cerr << "Failed to redirect stdout" << std::endl;
			exit(EXIT_FAILURE);
		}

		char *argv[] = { const_cast<char *>("/usr/bin/python3"),
						 const_cast<char *>(path.c_str()),
						 0 };
		if (execve("/usr/bin/python3", argv, 0) == -1) {
			std::cerr << "Failed to execute CGI script" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	else { // Parent process
		// Close the write end of the pipe
		close(pipe_fd[1]);

		// Read the output from the child process
		char buffer[128];
		std::string result;
		ssize_t bytes_read;
		while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
			result.append(buffer, bytes_read);
		}

		// Close the read end of the pipe
		close(pipe_fd[0]);

		// Wait for the child process to complete
		int status;
		waitpid(pid, &status, 0);

		// Check if the child process terminated successfully
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			std::cerr << "Child process failed" << std::endl;
			exit(EXIT_FAILURE);
		}

		return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " +
			intToString(result.length()) + "\r\n\r\n" + result;
	}
	return "";
}

std::string Cgi::run()
{
	std::string result = "";
	result = runCommand();

	return result;
}
