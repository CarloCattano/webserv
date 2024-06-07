#include "./ServerCluster.hpp"
#include "../Utils/utils.hpp"

const int MAX_EVENTS = 42;
const int BUFFER_SIZE = 4096; // TODO check pipe max buff

ServerCluster::ServerCluster(std::vector<Server> &servers)
{
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1)
		perror("epoll_create1");

	for (size_t i = 0; i < servers.size(); i++) {
		int socket_fd = servers[i].getSocketFd();

		_server_map[socket_fd] = servers[i];

		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = socket_fd;

		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
			perror("epoll_ctl");
			exit(EXIT_FAILURE);
		}
	}
}

int ServerCluster::accept_new_connection(int server_fd)
{
	int client_fd = accept(server_fd, NULL, NULL);

	if (client_fd == -1) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	return (client_fd);
}

void ServerCluster::handle_new_client_connection(int server_fd)
{
	int client_fd = accept_new_connection(server_fd);

	Client *client = new Client(client_fd, &_server_map[server_fd], _epoll_fd);

	_client_map[client_fd] = *client;
}

void ServerCluster::close_client(int fd)
{
	_client_map.erase(fd);
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
}

void ServerCluster::add_client_fd_to_epoll(int client_fd)
{
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = client_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}
}

int ServerCluster::get_client_fd_from_pipe_fd(int pipe_fd, std::map<int, int> &client_fd_to_pipe_map)
{
	for (std::map<int, int>::iterator it = client_fd_to_pipe_map.begin(); it != client_fd_to_pipe_map.end(); ++it) {
		if (it->second == pipe_fd) {
			return it->first;
		}
	}
	return -1;
}


void ServerCluster::await_connections()
{
	struct epoll_event events[MAX_EVENTS];
	int num_events;

	while (1) {
		num_events = epoll_wait(_epoll_fd, events, MAX_EVENTS, 500);
		if (num_events == -1)
			continue;

		for (int i = 0; i < num_events; i++) {
			int event_fd = events[i].data.fd;
			if (event_fd == -1) {
				perror("events[i].data.fd");
				continue;
			}

			bool is_pipe_fd = std::find(pipes.begin(), pipes.end(), event_fd) != pipes.end();

			if (is_pipe_fd) {
				int pipe_index = std::find(pipes.begin(), pipes.end(), event_fd) - pipes.begin();
				handle_pipe_event(event_fd, pipe_index);
			}
			else if (_server_map.count(event_fd)) {
				handle_new_client_connection(event_fd);
			}
			else {
				Client &client = _client_map[event_fd];

				if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, event_fd, NULL);
					close_client(event_fd);
				}

				if (events[i].events & EPOLLIN) {
					handle_request(client);
				}

				if (events[i].events & EPOLLOUT && client.getResponse().body.size() > 0) {
					handle_response(client);
				}
			}
		}
	}
}

void ServerCluster::handle_pipe_event(int pipe_fd, int pipe_index)

{
	char buffer[BUFFER_SIZE];
	int bytes_read = read(pipe_fd, buffer, BUFFER_SIZE);

	if (bytes_read == -1) {
		perror("handle_pipe_event read");
		close(pipe_fd);
		pipes.erase(pipes.begin() + pipe_index);
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, pipe_fd, NULL);
		return;
	}
	if (bytes_read > 0) {
		_cgi_response_map[pipe_fd] += std::string(buffer, bytes_read);
	}
	else if (bytes_read == 0) {
		std::string res = _cgi_response_map[pipe_fd];

		const int fd = get_client_fd_from_pipe_fd(pipe_fd, _client_fd_to_pipe_map);
		Client &client = _client_map[fd];

		_cgi_response_map.erase(pipe_fd);
		_client_fd_to_pipe_map.erase(fd);

		client.setResponseStatusCode(200);
		client.setResponseBody(res.c_str());
		client.addResponseHeader("Content-Length", intToString(res.size()));
		client.addResponseHeader("Content-Type", "text/html");
		client.addResponseHeader("Connection", "close");

		// TODO - get rind of pipe index
		pipes.erase(pipes.begin() + pipe_index);
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, pipe_fd, NULL);
		close(pipe_fd);
	}
}

void ServerCluster::switch_poll(int client_fd, uint32_t events)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = client_fd;

	if (client_fd == -1) {
		perror("client_fd");
		return;
	}

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
		perror("epoll_ctl");
		close(client_fd);
	}
}

void ServerCluster::handle_response(Client &client)
{
	Response response = client.getResponse();

	std::string response_string = client.responseToString();
	int bytes_sent = send(client.getFd(), response_string.c_str(), response_string.size(), 0);

	if (bytes_sent == -1) {
		perror("send");
		close_client(client.getFd());
		return;
	}

	client.setSentBytes(client.getSentBytes() + bytes_sent);

	if (client.getSentBytes() == response_string.size()) {
		close_client(client.getFd());
	}
}

void ServerCluster::stop(int signal)
{
	(void)signal;
	log("\nServer stopped");
	exit(0);
}

/* takes care of the signal when a child process is terminated
	and the parent process is not waiting for it
	so it doesn't become a zombie process */
void handleSigchild(int sig)
{
	(void)sig;
	while (waitpid(-1, NULL, WNOHANG) > 0)
		continue;
}

void ServerCluster::start()
{
	if (signal(SIGCHLD, handleSigchild) == SIG_ERR)
		perror("signal(SIGCHLD) error");

	signal(SIGINT, stop);
	ServerCluster::await_connections();
}

ServerCluster::~ServerCluster()
{
	// for (size_t i = 0; i < _servers.size(); i++) {
	//     close(_servers[i].getSocketFd());
	// }
}
