#include "Config.hpp"

Config::Config()
{
}

Config::~Config()
{
}

void parse_key_with_values(Virtual_Server_Config &virtual_server, std::string str, int i) {
	std::vector<std::string> key_with_values = convert_server_line_2_vector(str, i);
	int	size = key_with_values.size();
	std::string	key = key_with_values[0];

	if (key == "listen") {
		if (size >= 2)
			virtual_server.port = atoi(key_with_values[1].c_str());
		if (size >= 3 && key_with_values[2] == "default")
			virtual_server.default_server = true;
		else
			virtual_server.default_server = false;
	}
	else if (key == "server_name") {
		virtual_server.server_names = get_values(key_with_values);
	}
	else if (key == "error_page") {
		virtual_server.error_pages = get_values(key_with_values);
	}
	else if (key == "client_max_body_size" && size >= 2) {
		virtual_server.client_max_body_size = key_with_values[1];
	}
}

Virtual_Server_Config get_virtual_server_obj(std::string str, int i) {
	Virtual_Server_Config	virtual_server;

	i = iterate_to_first_server_line(str, i);
	while (str[i] && str[i] != '}') {
		if (str.substr(i, 8) == "location")
			i = parse_route(virtual_server, str, i);
		else
			parse_key_with_values(virtual_server, str, i);
		i = iterate_to_next_server_line(str, i);
	}
	if (str[i] != '}') {
		throw std::runtime_error("Ending curly brace missing: '}'");
	}
	return (virtual_server);
}

Config::Config(const std::string filename)
{
	Virtual_Server_Config virtual_server_obj;
	std::string file_content = get_file_content(filename);

	size_t index = file_content.find("server");
	while (index != std::string::npos) {
		virtual_server_obj = get_virtual_server_obj(file_content, index + 6);
		print_server_obj(virtual_server_obj, index);
		this->virtual_servers.push_back(virtual_server_obj);
		while (file_content[index] && file_content[index] != '}') {index++;}
		index = file_content.find("server", index);
	}
	// set default server
}

std::vector<Virtual_Server_Config> Config::get_virtual_servers()
{
	return (this->virtual_servers);
}