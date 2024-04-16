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

struct Method {
	bool	is_allowed;
	bool	can_be_edited;

	Method()
		: is_allowed(true), can_be_edited(true) {}
};

struct Route {
	std::string						location;
	std::string						matching_style;
	std::string						root;
	std::vector<HttpRedirection>	redirections;
	bool							autoindex;
	std::vector<std::string>		index_files;
	Method							POST;
	Method							GET;

	Route()
		: location(""), matching_style(""), root(""), autoindex(false) {
		redirections = std::vector<HttpRedirection>();
		index_files = std::vector<std::string>();
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
int	parse_route(Virtual_Server_Config &virtual_server, std::string str, int i);
int	iterate_to_next_server_line(std::string str, int i);
std::vector<std::string>	convert_server_line_2_vector(std::string str, int i);
int	iterate_to_first_server_line(std::string str, int i);
template<typename T>
std::vector<T> get_values(std::vector<T> key_with_values) {
	std::vector<T> values(key_with_values.begin() + 1, key_with_values.end());
	return (values);
}