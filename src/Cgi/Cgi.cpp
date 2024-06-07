#include "Cgi.hpp"
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../Utils/utils.hpp"

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

void Cgi::handle_cgi_request(Client &client,
							 const std::string &cgi_script_path,
							 std::vector<int> &pipes,
							 std::map<int, int> &_client_fd_to_pipe_map,
							 int _epoll_fd)
{
	int pipe_fd[2];
	int fd = client.getFd();

	if (pipe(pipe_fd) == -1) {
		perror("pipe2");
		return;
	}

	pid_t pid;

	if ((pid = fork()) == -1) {
		perror("fork");
		return;
	}

	if (pid == 0) {
		close(pipe_fd[0]);

		dup2(pipe_fd[1], STDOUT_FILENO);
		close(pipe_fd[1]);

		char *av[] = { const_cast<char *>("/usr/bin/python3"), strdup(cgi_script_path.c_str()), NULL };
		execve(av[0], av, NULL);
		exit(1);
	}
	else {
		close(pipe_fd[1]);

		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = pipe_fd[0];
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, pipe_fd[0], &ev) == -1) {
			perror("epoll_ctl");
			return;
		}

		pipes.push_back(pipe_fd[0]);
		_client_fd_to_pipe_map[fd] = pipe_fd[0];

		// TODO - REMOVE ?
		int status;
		if (waitpid(pid, &status, WNOHANG) > 0) {
			log("Cgi child done!");
			// clean up pipes and epoll
			close(pipe_fd[0]);
			pipes.pop_back();
			_client_fd_to_pipe_map.erase(fd);
			epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, pipe_fd[0], NULL);
			Error("pipe Done");
			return;
		}
	}
}
