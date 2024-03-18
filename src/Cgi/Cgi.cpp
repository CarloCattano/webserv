#include "Cgi.hpp"
#include <csignal>
#include <cstdlib>
#include <sys/wait.h>
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

std::string relativePath(std::string path)
{
	std::string prefix = "website/cgi-bin/";
	std::string result = prefix + path;
	return result;
}

std::string runCommand(const std::string &scriptPath)
{
	const int TIMEOUT_SECONDS = 20; // Adjust timeout as needed

	if (scriptPath.empty()) {
		throw std::invalid_argument("Empty script path");
	}

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

		alarm(TIMEOUT_SECONDS);

		if (execl("/usr/bin/python3", "python3", relativePath(scriptPath).c_str(), NULL) == -1) {
			std::cerr << "Failed to execute Python script" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	else {
		close(pipe_fd[1]);

		// Read the output from the child process
		char buffer[128];
		std::string result;
		ssize_t bytes_read;
		while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
			result.append(buffer, bytes_read);
		}

		close(pipe_fd[0]);

		int status;
		waitpid(pid, &status, 0);

		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			std::cerr << "Child process failed" << std::endl;
			exit(EXIT_FAILURE);
		}

		return result;
	}
	return "";
}

std::string Cgi::run(const std::string &scriptPath)
{
	std::string result = "";
	result = runCommand(scriptPath);
	return result;
}
