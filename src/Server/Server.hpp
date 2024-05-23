#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include "../Utils/FileUpload.hpp"

#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
// #include "Config.hpp"

struct HttpRedirection {
	std::string	code;
	std::string	url;

	HttpRedirection(std::string url)
		: code("302"), url(url) {}
};

struct Method {
	bool	is_allowed;
	bool	can_be_edited;

	Method()
		: is_allowed(true), can_be_edited(true) {}
};

struct Fastcgi_Param {
	std::string key;
	std::string value;

	Fastcgi_Param(std::string key, std::string value)
		: key(key), value(value) {}
};

struct Route {
	std::string						location;
	std::string						matching_style;
	std::string						root;
	std::vector<HttpRedirection>	redirections;
	bool							autoindex;
	std::vector<std::string>		index_files;
	// TO-DO add delete
	Method							POST;
	Method							GET;
	std::string						fastcgi_pass;
	std::string						fastcgi_index;
	std::vector<Fastcgi_Param>		fastcgi_params;

	Route()
		: location(""), matching_style(""), root(""), autoindex(false), fastcgi_pass(""), fastcgi_index("") {
		redirections = std::vector<HttpRedirection>();
		index_files = std::vector<std::string>();
		fastcgi_params = std::vector<Fastcgi_Param>();
	}
};

// to-do server only uses first server name
// Everything is public atm
class Server {
	private:

	public:
		unsigned int				_port;
		bool						_default_server;
		std::vector<std::string>	_server_names;
		std::vector<std::string>	_error_pages;
		std::string					_client_max_body_size;
		std::vector<Route>			_routes;

		int							_socket_fd;
		struct sockaddr_in			_server_address;
		Server();
		Server(unsigned int port, std::string host);

		~Server();
		void setup();
		int	getSocketFd();

		// in_addr_t getServerName() {
		// 	return inet_addr(_server_names[0].data());
		// };
};
