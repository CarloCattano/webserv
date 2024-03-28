#include "Server.hpp"
#include <string>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include "../Utils/utils.hpp"
#include "Cgi.hpp"

const int MAX_EVENTS = 100;
const int BACKLOG = 20;
const int BUFFER_SIZE = 1024;

// get the path of the folder from where the server is run

std::string get_current_dir()
{
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		return std::string(cwd);
	else
		return "";
}

std::string CGI_BIN = get_current_dir() + "/website/cgi-bin/" + "test.py"; // TODO load from config

Server::Server(std::string ip_address, int port) : _ip_address(ip_address), _port(port)
{
	_server_address.sin_family = AF_INET;
	_server_address.sin_addr.s_addr = INADDR_ANY;
	_server_address.sin_port = htons(_port);
	start();
}

void Server::stop(int signal)
{
	(void)signal;
	log("\nServer stopped");
	exit(0);
}

// takes care of the signal when a child process is terminated
// and the parent process is not waiting for it
// so it doesn't become a zombie process

void handleSigchild(int sig)
{
	(void)sig;
	while (waitpid(-1, NULL, WNOHANG) > 0)
		continue;
}

void Server::start_listen()
{
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd == -1)
		throw SocketErrorException();

	int opt = 1;
	setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

	fcntl(_socket_fd, F_SETFL, O_NONBLOCK);

	if (bind(_socket_fd, (struct sockaddr *)&_server_address, sizeof(_server_address)) == -1)
		throw BindErrorException();

	if (listen(_socket_fd, BACKLOG) == -1)
		throw ListenErrorException();
}

void Server::await_connections()
{
	/* fds[0].fd = _socket_fd; */
	/* fds[0].events = POLLIN; */
	/* std::cout << "Server started on http://localhost:" << _port << std::endl; */

	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = _socket_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _socket_fd, &ev) == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}

	while (1) {
		struct epoll_event events[MAX_EVENTS];
		int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (num_events == -1) {
			continue;
		}

		for (int i = 0; i < num_events; ++i) {
			if (events[i].data.fd == _socket_fd) {
				int client_fd = accept(_socket_fd, NULL, NULL);
				if (client_fd == -1) {
					perror("accept");
					continue;
				}
				fcntl(client_fd, F_SETFL, O_NONBLOCK);
				ev.events = EPOLLIN | EPOLLET; // Add client socket to epoll
				ev.data.fd = client_fd;

				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
					perror("epoll_ctl");
					exit(EXIT_FAILURE);
				}
			}
			else {
				int client_fd = events[i].data.fd;
				handle_request(client_fd);
				close(client_fd);
			}
		}
	}
}

void Server::start()
{
	if (signal(SIGCHLD, handleSigchild) == SIG_ERR)
		perror("signal(SIGCHLD) error");

	signal(SIGINT, stop);
	start_listen();
	await_connections();
}

void Server::handle_request(int client_fd)
{
	char buffer[BUFFER_SIZE];
	int size = recv(client_fd, buffer, BUFFER_SIZE, 0); // MSG_DONTWAIT);

	if (size == -1) {
		perror("recv");
		return;
	}

	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	std::string requested_file_path = extract_requested_file_path(buffer);
	std::string file_content = readFileToString("website" + requested_file_path);
	std::string content_type = getContentType(requested_file_path);

	if (requested_file_path.find(".py") != std::string::npos) {
		// TODO check if file exists and we are allowed to execute it
		//      from the config file
		// TODO check if the file is executable provided by config only

		pid_t pid = fork();

		if (pid == -1) {
			perror("fork");
			return;
		}

		if (pid == 0) {
			// Child process - for every request
			try {
				close(_socket_fd); // Close the listening socket in the child process

				Cgi cgi;

				std::string cgi_response = cgi.run(CGI_BIN);

				std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type +
					"\r\nContent-Length: " + intToString(cgi_response.length()) + "\r\n\r\n" +
					cgi_response.c_str();

				send(client_fd, response.c_str(), response.size(), 0);
				close(client_fd);
				exit(0); // Exit the child process
			}
			catch (std::exception &e) {
				std::cerr << "Error: " << e.what() << std::endl;
				exit(1);
			}
		}
		else {
			// Close the client socket in the parent process and continue accepting connections
			close(client_fd);
		}
	}
	else if (file_content.empty()) {
		std::string errResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
		send(client_fd, errResponse.c_str(), errResponse.size(), 0);
		close(client_fd);
	}
	else {
		std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type +
			"\r\nContent-Length: " + intToString(file_content.length()) + "\r\n\r\n" + file_content;

		send(client_fd, response.c_str(), response.size(), 0);
		close(client_fd);
	}
}

Server::~Server()
{
	close(_socket_fd);
}
