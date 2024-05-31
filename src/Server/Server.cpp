#include "./Server.hpp"
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <ostream>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

// include for exit function
#include <stdlib.h>

Server::Server()
	: _port(8000), _default_server(false), _server_names(1, "0.0.0.0"), _client_max_body_size(1073741824), _autoindex(false), _root("/www/website1"), _cgi_path(""), _cgi_extension("") {
	// _server_names = std::vector<std::string>();
	_error_pages = std::vector<std::string>();
	_routes = std::vector<Route>();
}

Server::Server(const Server &server) {
	*this = server;
}

//operator overload
Server &Server::operator=(const Server &server) {
	if (this == &server)
		return *this;
	_port = server._port;
	_default_server = server._default_server;
	_server_names = server._server_names;
	_error_pages = server._error_pages;
	_client_max_body_size = server._client_max_body_size;
	_routes = server._routes;
	_autoindex = server._autoindex;
	_server_address = server._server_address;
	_socket_fd = server._socket_fd;
	return *this;
}

Server::~Server() {}

void Server::setup() {

	// NEED TO ADJUST EXITS
	if ((_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
		perror("socket");
	int option_value = 1;

	setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));

	_server_address.sin_family = AF_INET;
	_server_address.sin_addr.s_addr = inet_addr(_server_names[0].data());
	_server_address.sin_port = htons(_port);

	// TODO decide on an exit strategy
	if (bind(_socket_fd, (struct sockaddr *)&_server_address, sizeof(_server_address)) == -1) {
		std::cerr << "Error: " << _port << " is already in use, exiting .... \n----\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	if (listen(_socket_fd, SOMAXCONN) == -1)
		perror("listen"); // EXIT FAILURE ?

	std::cout << "Server started at http://" << _server_names[0] << ":" << _port << std::endl;
}

//getters
int Server::getSocketFd() { return this->_socket_fd; }

unsigned int Server::getPort() { return this->_port; }

std::vector<Route> Server::getRoutes() { return this->_routes; }

std::vector<std::string> Server::getServerNames() { return this->_server_names; }

std::vector<std::string> Server::getErrorPages() { return this->_error_pages; }

long long Server::getClientMaxBodySize() { return this->_client_max_body_size; }

struct sockaddr_in Server::getServerAddress() { return this->_server_address; }

bool Server::getDefaultServer() { return this->_default_server; }

bool Server::getAutoindex() { return this->_autoindex; }

std::string Server::getRoot() { return this->_root; }

Method Server::getGet() { return this->_GET; }

Method Server::getPost() { return this->_POST; }

Method Server::getDelete() { return this->_DELETE; }

std::string Server::getCgiPath() { return this->_cgi_path; }

std::string Server::getCgiExtension() { return this->_cgi_extension; }

//setters
void Server::setPort(unsigned int port) { this->_port = port; }

void Server::setDefaultServer(bool default_server) { this->_default_server = default_server; }

void Server::setServerNames(std::vector<std::string> server_names) { this->_server_names = server_names; }

void Server::setErrorPages(std::vector<std::string> error_pages) { this->_error_pages = error_pages; }

void Server::setClientMaxBodySize(long long client_max_body_size) { this->_client_max_body_size = client_max_body_size; }

void Server::setAutoindex(bool autoindex) { this->_autoindex = autoindex; }

void Server::setSocketFd(int socket_fd) { this->_socket_fd = socket_fd; }

void Server::setServerAddress(struct sockaddr_in server_address) { this->_server_address = server_address; }

void Server::addRoute(Route route) { this->_routes.push_back(route); }

void Server::setRoutes(std::vector<Route> routes) { this->_routes = routes; }

void Server::setRoot(std::string root) { this->_root = root; }

void Server::setGet(Method method) { this->_GET = method; }

void Server::setPost(Method method) { this->_POST= method; }

void Server::setDelete(Method method) { this->_DELETE= method; }

void Server::setCgiPath(std::string path) { this->_cgi_path = path; }

void Server::setCgiExtension(std::string extension) { (void)extension; this->_cgi_extension = "test"; }
