#include "Config.hpp"

Config::Config()
{
}

Config::~Config()
{
}

void parse_listen(int size, Server &server, std::vector<std::string> &key_with_values)
{
	if (size >= 2)
		server._port = atoi(key_with_values[1].c_str());
	if (size >= 3 && key_with_values[2] == "default")
		server._default_server = true;
	else
		server._default_server = false;
}

void parse_key_with_values(Server &server, std::string str, int i) {
	std::vector<std::string> key_with_values = convert_server_line_2_vector(str, i);
	int	size = key_with_values.size();
	std::string	key = key_with_values[0];

	if (key == "listen")
		parse_listen(size, server, key_with_values);
	else if (key == "server_name")
		server._server_names = extract_values(key_with_values);
	else if (key == "error_page")
		server._error_pages = extract_values(key_with_values);
	else if (key == "client_max_body_size" && size >= 2)
		server._client_max_body_size = key_with_values[1];
}

Server get_server_obj(std::string str, int i) {
	Server	server;

	i = iterate_to_first_server_line(str, i);
	while (str[i] && str[i] != '}') {
		if (str.substr(i, 8) == "location")
			i = parse_route(server, str, i);
		else
			parse_key_with_values(server, str, i);
		i = iterate_to_next_server_line(str, i);
	}
	if (str[i] != '}') {
		throw std::runtime_error("Ending curly brace missing: '}'");
	}
	return (server);
}

Config::Config(const std::string filename)
{
	Server		server_obj;
	// std::string	file_content = get_file_content(filename);
	(void)filename;
	// std::cout << file_content << std::endl;

	// size_t index = file_content.find("server");
	// while (index != std::string::npos) {
	// 	server_obj = get_server_obj(file_content, index + 6);
	// 	print_server_obj(server_obj, index);
	// 	this->_servers.push_back(server_obj);
	// 	while (file_content[index] && file_content[index] != '}') {index++;}
	// 	index = file_content.find("server", index);
	// }
	// set default server
}

std::vector<Server> Config::get_servers()
{
	return (this->_servers);
}