#include "Config.hpp"

Config::Config()
{
}

Config::~Config()
{

}

template<typename T>
std::vector<T> get_values(std::vector<T> key_with_values) {
	std::vector<T> values(key_with_values.begin() + 1, key_with_values.end());
	return (values);
}

int	iterate_to_first_server_line(std::string str, int i) {
	while (is_whitespace_char(str[i])) {i++;}
	if (str[i++] != '{') {
		throw std::runtime_error("Starting curly brace missing: '{'");
	}
	while (is_whitespace_char(str[i])) {i++;}
	return (i);
}

int	iterate_to_next_server_line(std::string str, int i) {
	while (str[i] && str[i] != '}' && str[i] != ';' && str[i] != '\n') {
		i++;
	}
	while (str[i] && str[i] != '}' && (str[i] == ';' || is_whitespace_char(str[i]))) {
		i++;
	}
	return (i);
}

std::vector<std::string>	convert_server_line_2_vector(std::string str, int i) {
	std::vector<std::string> values;

	while (str[i] && str[i] != '}' && str[i] != ';' && str[i] != '\n') {
		std::string single_value;
		while (str[i] && str[i] != '}' && str[i] != ';' && str[i] != '\n' && str[i] != ' ') {
			single_value+= str[i++];
		}
		values.push_back(single_value);
		while (str[i] == ' ') {i++;}
	}
	return (values);
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

int	parse_route(Virtual_Server_Config &virtual_server, std::string str, int i) {
	Route						route;
	std::vector<std::string> 	key_with_values;
	int							size;

	// i = iterate_to_first_server_line(str, i);
	while (str[i] && str[i] != '}') {
		key_with_values = convert_server_line_2_vector(str, i);
		size = key_with_values.size();

		if (key_with_values[0] == "location") {
			if (size < 3 || size > 4 || key_with_values[size - 1] != "{")
				throw std::runtime_error("Bad route format on first line.");
			if (size == 4)
				route.matching_style = key_with_values[1];
			route.location = key_with_values[size - 2];
		}
		else if (key_with_values[0] == "root" && size == 2)
			route.root = key_with_values[1];
		else if (key_with_values[0] == "return" && size >= 2 && size <= 3) {
			HttpRedirection redirection(key_with_values[size - 1]);
			if (size == 3) {
				redirection.code = key_with_values[1];
			}
			route.redirections.push_back(redirection);
		}
		else if (key_with_values[0] == "autoindex" && size == 2 && toLowerCase(key_with_values[1]) == "true")
			route.autoindex = true;
		i = iterate_to_next_server_line(str, i);
	}
	if (str[i] == '}')
		i++;
	virtual_server.routes.push_back(route);
	return (i);
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