#include "./Server.hpp"
#include <iostream>
#include <ostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

Server::Server()
	: _port(8000), _default_server(false), _server_names(1, "0.0.0.0"), _client_max_body_size(1073741824),
	  _autoindex(false), _index_file("index.html"), _root("/www/berlin-forecast"), _redirection(HttpRedirection()),
	  _cgi_path(""), _cgi_extension("") {
	_error_pages = std::vector<std::string>();
	_routes = std::vector<Route>();
}

Server::Server(const Server &server) {
	*this = server;
}

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
	_index_file = server._index_file;
	_socket_fd = server._socket_fd;
	_root = server._root;
	_redirection = server._redirection;
	_GET = server._GET;
	_POST = server._POST;
	_DELETE = server._DELETE;
	_cgi_path = server._cgi_path;
	_cgi_extension = server._cgi_extension;
	return *this;
}

Server::~Server() {
}

void Server::setup() {
	if ((_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
		std::cerr << "Error: socket creation failed" << std::endl;

	int option_value = 1;

	setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));

	sockaddr_in server_address;

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(_server_names[0].data());
	server_address.sin_port = htons(_port);

	if (bind(_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
		std::cerr << "Error: " << _port << " is already in use, exiting .... \n----\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	if (listen(_socket_fd, SOMAXCONN) == -1) {
		std::cerr << "Error: listen failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "Server started at http://" << _server_names[0] << ":" << _port << std::endl;
}

// getters
int Server::getSocketFd() {
	return this->_socket_fd;
}

unsigned int Server::getPort() {
	return this->_port;
}

std::vector<Route> Server::getRoutes() {
	return this->_routes;
}

std::vector<std::string> Server::getServerNames() {
	return this->_server_names;
}

std::vector<std::string> Server::getErrorPages() {
	return this->_error_pages;
}

long long Server::getClientMaxBodySize() {
	return this->_client_max_body_size;
}

bool Server::getDefaultServer() {
	return this->_default_server;
}

bool Server::getAutoindex(std::string *location) {
	Route *route = location ? get_route(*location) : NULL;

	if (route)
		return (route->autoindex);
	else
		return this->_autoindex;
}


std::string Server::getRoot(std::string *location) {
	Route *route = location ? get_route(*location) : NULL;

	if (route && route->root != "")
		return (route->root);
	else
		return this->_root;
}

HttpRedirection Server::getRedirection(std::string *location) {
	Route *route = location ? get_route(*location) : NULL;

	if (route && route->redirection.code)
		return (route->redirection);
	else
		return this->_redirection;
}

Route *Server::get_route(std::string location) {
	if (location[location.size() - 1] != '/')
		location += '/';
	for (size_t i = 0; i < _routes.size(); ++i) {
		if (_routes[i].location == location)
			return (&_routes[i]);
	}
	return (NULL);
}

Method Server::getGet(std::string *location) {
	Route *route = location ? get_route(*location) : NULL;

	if (route)
		return (route->GET);
	else
		return this->_GET;
}

Method Server::getPost(std::string *location) {
	Route *route = location ? get_route(*location) : NULL;

	if (route)
		return (route->POST);
	else
		return this->_POST;
}

Method Server::getDelete(std::string *location) {
	Route *route = location ? get_route(*location) : NULL;

	if (route)
		return (route->DELETE);
	else
		return this->_DELETE;
}

std::string Server::getIndexFile(std::string *location) {
	Route *route = location ? get_route(*location) : NULL;

	if (route)
		return (route->index_file);
	else
		return this->_index_file;
}

std::string Server::get_full_path(std::string location) {
	Route *route = get_route(location);
	std::string full_path = "." + this->getRoot(&location);

	if (!route || (route && route->root == ""))
		full_path += location;
	return (full_path);
}

std::string Server::getCgiPath() {
	return this->_cgi_path;
}

std::string Server::getCgiExtension() {
	return this->_cgi_extension;
}

// setters
void Server::setPort(unsigned int port) {
	this->_port = port;
}

void Server::setDefaultServer(bool default_server) {
	this->_default_server = default_server;
}

void Server::setServerNames(std::vector<std::string> server_names) {
	this->_server_names = server_names;
}

void Server::setErrorPages(std::vector<std::string> error_pages) {
	this->_error_pages = error_pages;
}

void Server::setClientMaxBodySize(long long client_max_body_size) {
	this->_client_max_body_size = client_max_body_size;
}

void Server::setAutoindex(bool autoindex) {
	this->_autoindex = autoindex;
}

void Server::setSocketFd(int socket_fd) {
	this->_socket_fd = socket_fd;
}

void Server::setServerAddress(struct sockaddr_in server_address) {
	this->_server_address = server_address;
}

void Server::addRoute(Route route) {
	this->_routes.push_back(route);
}

void Server::setRoutes(std::vector<Route> routes) {
	this->_routes = routes;
}

void Server::setIndexFile(std::string index_file_name) {
	this->_index_file = index_file_name;
}

void Server::setRoot(std::string root) {
	this->_root = root;
}

void Server::setRedirection(int code, std::string url) {
	this->_redirection.code = code;
	this->_redirection.url = url;
}

void Server::setGet(Method method) {
	this->_GET = method;
}

void Server::setPost(Method method) {
	this->_POST = method;
}

void Server::setDelete(Method method) {
	this->_DELETE = method;
}

void Server::setCgiPath(std::string path) {
	this->_cgi_path = path;
}

void Server::setCgiExtension(std::string extension) {
	this->_cgi_extension = extension;
}
