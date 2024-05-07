#include "Cgi.hpp"
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
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
	// add main folder abs path
	std::string mainFolder = "/home/carlo/42/webserv/";
	std::string prefix = "website/cgi-bin/"; // TODO : parse from config
	std::string result = mainFolder + prefix + path;
	return result;
}

/**
 * c++98 must be used and popen is forbiden, so this is the reason why we use fork and exec
 * the problem is that waitpid is blocking the process, so we need
 * to find a way to make it non-blocking
 **/

std::string runCommand(const std::string &scriptPath)
{
	const int TIMEOUT_SECONDS = 20;
	std::string result = "";
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

	if (pid == 0) {
		close(pipe_fd[0]); // Close read

		// stdout to write
		if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
			std::cerr << "Failed to redirect stdout" << std::endl;
			exit(EXIT_FAILURE);
		}

		alarm(TIMEOUT_SECONDS);

		char pyBin[] = "/usr/bin/python3";

		char *av[] = { pyBin, strdup(scriptPath.c_str()), NULL };

		if (execve(pyBin, av, NULL) == -1) {
			std::cerr << "Failed to execute Python script" << std::endl;
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}
	else {
		close(pipe_fd[1]);

		char buffer[1024]; // output from the child process

		ssize_t bytes_read;

		while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
			result.append(buffer, bytes_read);
		}

		close(pipe_fd[0]);

		int status;

		// waitpid is blocking the process, so we need to find a way to make it non-blocking
		// we can use WNOHANG flag to make it non-blocking
		// == 0 means that the child process is still running and we need to wait but we continue
		// the loop
		// != 0 means that the child process is done and we can return the result
		while (waitpid(pid, &status, WNOHANG) == 0) {
			continue;
		}
	}
	return result;
}

std::string Cgi::run(const std::string &scriptPath)
{
	std::string result = "";
	result = runCommand(scriptPath);
	return result;
}
