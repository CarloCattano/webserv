#pragma once

#include "../Utils/FileUpload.hpp"
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <vector>

struct HttpRedirection {
	std::string code;
	std::string url;

	HttpRedirection(std::string url) : code("302"), url(url) {}
};

struct Method {
	bool is_allowed;
	bool can_be_edited;

	Method() : is_allowed(true), can_be_edited(true) {}
	Method(bool is_allowed, bool can_be_edited)
		: is_allowed(is_allowed), can_be_edited(can_be_edited) {}
};

struct Fastcgi_Param {
	std::string key;
	std::string value;

	Fastcgi_Param(std::string key, std::string value) : key(key), value(value) {}
};

struct Route {
	std::string location;
	std::string matching_style;
	std::string root;
	std::vector<HttpRedirection> redirections;
	bool autoindex;
	std::vector<std::string> index_files;
	Method POST;
	Method GET;
	Method DELETE;

	Route() : location(""), matching_style(""), root(""), autoindex(false) {
		redirections = std::vector<HttpRedirection>();
		index_files = std::vector<std::string>();
	}
};

// to-do server only uses first server name
// Everything is public atm
class Server {
  private:
	unsigned int _port;
	bool _default_server;
	std::vector<std::string> _server_names;
	std::vector<std::string> _error_pages;
	long long _client_max_body_size;
	std::vector<Route> _routes;
	bool _autoindex;
	std::string _root;
	Method _POST;
	Method _GET;
	Method _DELETE;
	std::string _cgi_path;
	std::string _cgi_extension;
	int _socket_fd;
	struct sockaddr_in _server_address;

  public:
	Server();
	Server(const Server &server);
	Server &operator=(const Server &server);
	~Server();

	unsigned int getPort();
	bool getDefaultServer();
	std::vector<std::string> getServerNames();
	std::vector<std::string> getErrorPages();
	long long getClientMaxBodySize();
	std::vector<Route> getRoutes();
	bool getAutoindex();
	int getSocketFd();
	struct sockaddr_in getServerAddress();
	std::string getRoot();
	Method getGet();
	Method getPost();
	Method getDelete();
	std::string getCgiPath();
	std::string getCgiExtension();

	// setters
	void setPort(unsigned int port);
	void setDefaultServer(bool default_server);
	void setServerNames(std::vector<std::string> server_names);
	void setErrorPages(std::vector<std::string> error_pages);
	void setClientMaxBodySize(long long client_max_body_size);
	void setRoutes(std::vector<Route> routes);
	void addRoute(Route route);
	void setAutoindex(bool autoindex);
	void setSocketFd(int socket_fd);
	void setServerAddress(struct sockaddr_in server_address);
	void setRoot(std::string root);
	void setGet(Method method);
	void setPost(Method method);
	void setDelete(Method method);
	void setCgiPath(std::string path);
	void setCgiExtension(std::string extension);

	void setup();
};
