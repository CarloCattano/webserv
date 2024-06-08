#pragma once

#include "../Server/Server.hpp"
#include <stdlib.h>

class Config {
  private:
	std::string 		_filename;
	std::vector<Server> _servers;
	// Server				default_server;

  public:
	Config();
	~Config();
	Config(const std::string filename);
	std::vector<Server>& get_servers();
};

std::string								get_file_content(const std::string &filename);
bool 									is_whitespace_char(char c);
void 									print_server_obj(Server &obj);
template <typename T> void 				print_vector(T vector);
std::string 							toLowerCase(std::string str);
int 									parse_route(Server &server, std::string str, int i);
int 									iterate_to_next_server_line(std::string str, int i);
std::vector<std::string>				convert_server_line_2_vector(std::string str, int i);
int 									iterate_to_first_server_line(std::string str, int i);
template <typename T> std::vector<T>	extract_values(std::vector<T> key_with_values) {
	std::vector<T> values(key_with_values.begin() + 1, key_with_values.end());
	return (values);
}
bool 									isNumeric(const std::string& str);
