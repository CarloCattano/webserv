#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

struct HttpRedirection {
	std::string	code;
	std::string	url;

	HttpRedirection(std::string url)
		: code("302"), url(url) {}
};

struct Route {
	std::string						location;
	std::string						matching_style;
	std::string						root;
	std::vector<HttpRedirection>	redirections;
	bool							autoindex;

	Route()
		: location(""), matching_style(""), root(""), autoindex(false) {
		redirections = std::vector<HttpRedirection>();
	}
};

struct Virtual_Server_Config {
	unsigned int				port;
	bool						default_server;
	std::vector<std::string>	server_names;
	std::vector<std::string>	error_pages;
	std::string					client_max_body_size;
	std::vector<Route>			routes;

	Virtual_Server_Config()
		: port(0), default_server(false), client_max_body_size("") {
		server_names = std::vector<std::string>();
		error_pages = std::vector<std::string>();
		routes = std::vector<Route>();
	}
};

class Config {
public:
	Config();
	~Config();
	Config(const std::string filename);
	std::vector<Virtual_Server_Config> get_virtual_servers();

private:
	std::string 						filename;
	std::vector<Virtual_Server_Config> 	virtual_servers;
	Virtual_Server_Config				default_server;
};

std::string get_file_content(const std::string &filename);
bool is_whitespace_char(char c);
void	print_server_obj(Virtual_Server_Config obj, int i);
template <typename T>
void    print_vector(T vector);
std::string toLowerCase(std::string str);