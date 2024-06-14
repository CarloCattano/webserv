#include "./ServerCluster.hpp"
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../Utils/utils.hpp"

#define error(x) std::cerr << RED << x << RESET << std::endl

const int MAX_EVENTS = 1024;
const int PIPE_BUFFER_SIZE = 65536;

volatile static sig_atomic_t gSigStatus;

ServerCluster::ServerCluster(std::vector<Server> &servers) {
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1)
		error("epoll_create1");

	for (size_t i = 0; i < servers.size(); i++) {
		int socket_fd = servers[i].getSocketFd();

		_server_map[socket_fd] = servers[i];

		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = socket_fd;

		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
			error("epoll_ctl");
			exit(EXIT_FAILURE);
		}
	}
}

int ServerCluster::accept_new_connection(int server_fd) {
	int client_fd = accept(server_fd, NULL, NULL);

	if (client_fd == -1) {
		error("accept");
		exit(EXIT_FAILURE);
	}

	return (client_fd);
}

void ServerCluster::handle_new_client_connection(int server_fd) {
	int client_fd = accept_new_connection(server_fd);

	if (client_fd != -1) {
		Client *client = new Client(client_fd, &_server_map[server_fd], _epoll_fd);
		_client_map[client_fd] = client;
	}
}

void ServerCluster::close_client(int fd) {
	delete _client_map[fd];
	_client_map.erase(fd);
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
}

void ServerCluster::add_client_fd_to_epoll(int client_fd) {
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = client_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
		error("epoll_ctl");
		exit(EXIT_FAILURE);
	}
}

void ServerCluster::await_connections() {
	struct epoll_event events[MAX_EVENTS];
	int num_events;

	gSigStatus = 1;

	while (gSigStatus) {
		num_events = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
		if (num_events == -1)
			continue;

		for (int i = 0; i < num_events; i++) {
			int event_fd = events[i].data.fd;

			if (event_fd == -1) {
				error("events[i].data.fd");
				continue;
			}

			if (_pipe_client_map.find(event_fd) != _pipe_client_map.end()) {
				handle_pipe_event(event_fd);
			} else if (_server_map.count(event_fd)) {
				handle_new_client_connection(event_fd);
			} else {
				Client *client = _client_map[event_fd];

				check_timeout(*client, 5);

				if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR)
					close_client(event_fd);

				if (events[i].events & EPOLLIN)
					handle_request(*client);

				if (events[i].events & EPOLLOUT) //&& client->getResponse().body.size() > 0)
					handle_response(*client);
				log_open_clients(_client_map);
			}
		}
	}
}

void ServerCluster::handle_pipe_event(int pipe_fd)

{
	char buffer[PIPE_BUFFER_SIZE];
	int bytes_read = read(pipe_fd, buffer, PIPE_BUFFER_SIZE);

	if (bytes_read == -1) {
		error("handle_pipe_event read");
		close(pipe_fd);
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, pipe_fd, NULL);
		return;
	}
	if (bytes_read > 0) {
		_cgi_response_map[pipe_fd] += std::string(buffer, bytes_read);
	} else if (bytes_read == 0) {
		std::string res = _cgi_response_map[pipe_fd];

		Client *client = _client_map[_pipe_client_map[pipe_fd]];

		_cgi_response_map.erase(pipe_fd);
		_pipe_client_map.erase(pipe_fd);

		client->setResponseStatusCode(200);
		client->setResponseBody(res.c_str());
		client->addResponseHeader("Content-Length", intToString(res.size()));
		client->addResponseHeader("Content-Type", "text/html");
		client->addResponseHeader("Connection", "close");

		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, pipe_fd, NULL);
		close(pipe_fd);
	}
}

void ServerCluster::switch_poll(int client_fd, uint32_t events) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = client_fd;

	if (client_fd == -1) {
		error("client_fd");
		return;
	}

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
		error("epoll_ctl");
		close_client(client_fd);
	}
}

void ServerCluster::handle_response(Client &client) {
	Response response = client.getResponse();

	std::string response_string = client.responseToString();
	int bytes_sent = send(client.getFd(), response_string.c_str(), response_string.size(), 0);

	if (bytes_sent == -1) {
		error("send");
		close_client(client.getFd());
		return;
	}

	client.setSentBytes(client.getSentBytes() + bytes_sent);

	if (client.getSentBytes() == response_string.size()) {
		close_client(client.getFd());
	}
}

void ServerCluster::stop(int signal) {
	(void)signal;
	gSigStatus = 0;
}

/**
 * @brief A _"gream reaper"_ function that waits for child processes to terminate
 *          and prevents them from becoming zombie processes.
 *          by calling waitpid with WNOHANG flag, it will return immediately if no child
 *          process has terminated. If a child process has terminated, it will be reaped
 *          and the parent process will continue to run.
 * @param sig the signal that is being handled
 */
void handleSigchild(int sig) {
	(void)sig;
	while (waitpid(-1, NULL, WNOHANG) > 0)
		continue;
}

void ServerCluster::start() {
	if (signal(SIGCHLD, handleSigchild) == SIG_ERR)
		error("signal(SIGCHLD) error");

	signal(SIGINT, stop);
	ServerCluster::await_connections();
}

void ServerCluster::check_timeout(Client &client, int timeout) {
	std::map<int, int> pid_start_time_map = client.getPidStartTimeMap();
	std::map<int, int>::iterator it = pid_start_time_map.begin();

	while (it != pid_start_time_map.end()) {
		if (time(NULL) - it->second > timeout) {
			client.sendErrorPage(504);
			close(client.getPidPipefdMap()[it->first]);
			kill(it->first, SIGKILL);
			client.removePidStartTimeMap(it->first);
			_pipe_client_map.erase(client.getPidPipefdMap()[it->first]);
			epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client.getPidPipefdMap()[it->first], NULL);
		}
		it++;
	}
}

int ServerCluster::get_pipefd_from_clientfd(int client_fd) {
	return _pipe_client_map[client_fd];
}

ServerCluster::~ServerCluster() {
	for (std::map<int, int>::iterator it = _pipe_client_map.begin(); it != _pipe_client_map.end(); it++) {
		close(it->first);
	}
	_pipe_client_map.clear();

	if (_cgi_response_map.size() > 0) {
		for (std::map<int, std::string>::iterator it = _cgi_response_map.begin(); it != _cgi_response_map.end(); it++) {
			close(it->first);
		}
	}

	for (std::map<int, Client *>::iterator it = _client_map.begin(); it != _client_map.end(); it++) {
		close(it->first);
		delete it->second;
	}

	for (std::map<int, Server>::iterator it = _server_map.begin(); it != _server_map.end(); it++) {
		close(it->first);
	}

	close(_epoll_fd);
}
