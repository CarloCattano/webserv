#include "Cgi.hpp"
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>

Cgi::Cgi() {
}

Cgi::~Cgi() {
}

Cgi &Cgi::operator=(const Cgi &src) {
	if (this != &src) {
		this->_cgi = src._cgi;
	}
	return *this;
}

Cgi::Cgi(const Cgi &src) {
	*this = src;
}

void Cgi::handle_cgi_request(Client &client, const std::string &cgi_script_path, std::map<int, int> &_pipe_client_map,
							 int epoll_fd) {
	int pipe_fd[2];
	int client_fd = client.getFd();

	if (pipe(pipe_fd) == -1) {
		return;
	}

	pid_t pid;

	if ((pid = fork()) == -1) {
		std::cerr << "fork failed" << std::endl;
		return;
	}

	if (pid == 0) {
		close(pipe_fd[0]);

		dup2(pipe_fd[1], STDOUT_FILENO);
		close(pipe_fd[1]);

		char *av[] = { const_cast<char *>("/usr/bin/python3"), strdup(cgi_script_path.c_str()), NULL };
		int ret;
		ret = execve(av[0], av, NULL);
		if (ret == -1)
			std::cerr << "execve failed" << std::endl;
		exit(1);
	} else {
		close(pipe_fd[1]);

		client.addPidStartTimeMap(pid, time(NULL));
		client.addPidPipefdMap(pid, pipe_fd[0]);
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = pipe_fd[0];
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_fd[0], &ev) == -1) {
			std::cerr << "epoll_ctl failed" << std::endl;
			return;
		}

		_pipe_client_map[pipe_fd[0]] = client_fd;

		waitpid(pid, NULL, WNOHANG);
	}
}
