#include "Config.hpp"
#include <stdlib.h>
#include <iostream>

Config::Config() {}

Config::~Config() {}

void    set_allowed_methods(Server &server, std::vector<std::string> key_with_values) {
	Method POST = server.getPost(NULL);
	Method GET = server.getGet(NULL);
	Method DELETE = server.getDelete(NULL);
    bool new_state;

    if (key_with_values[0] == "allow")
        new_state = true;
    else if (key_with_values[0] == "deny")
        new_state = false;
    else
        return;

    size_t  i = 1;
    while (i < key_with_values.size()) {
        std::string key = key_with_values[i];
        if (key == "all") {
            if (POST.can_be_edited)
                server.setPost(Method(new_state, true));
            if (GET.can_be_edited)
                server.setGet(Method(new_state, true));
            if (DELETE.can_be_edited)
                server.setDelete(Method(new_state, true));
        }
        else if (key == "POST" && POST.can_be_edited)
			server.setPost(Method(new_state, false));
        else if (key == "GET" && GET.can_be_edited)
			server.setGet(Method(new_state, false));
        else if (key == "DELETE" && DELETE.can_be_edited)
			server.setDelete(Method(new_state, false));

        i++;
    }
}

void parse_listen(int size, Server &server, std::vector<std::string> &key_with_values) {
	if (size >= 2)
		server.setPort(atoi(key_with_values[1].c_str()));
	server.setDefaultServer(size >= 3 && key_with_values[2] == "default" ? true : false);
}

void parse_client_max_body_size(int size, Server &server, std::vector<std::string> &key_with_values) {
	std::string	value;
	int			length;
	int 		unit = 1;

	if (size < 2)
		return;
	value = key_with_values[1];
	length = value.size();
	if (value[length - 1] == 'k' || value[length - 1] == 'K')
		unit =  1024;
	else if (value[length - 1] == 'm' || value[length - 1] == 'M')
		unit =  1048576;
	else if (value[length - 1] == 'g' || value[length - 1] == 'G')
		unit =  1073741824;
	if (unit > 1)
		value.erase(value.size() - 1);
	if (isNumeric(value))
		server.setClientMaxBodySize(atoll(value.c_str()) * unit);
}

void parse_key_with_values(Server &server, std::string str, int i) {
	std::vector<std::string> key_with_values = convert_server_line_2_vector(str, i);
	int size = key_with_values.size();
	std::string key = key_with_values[0];

	if (key == "listen")
		parse_listen(size, server, key_with_values);
	else if (key == "server_name")
		server.setServerNames(extract_values(key_with_values));
	else if (key == "error_page")
		server.setErrorPages(extract_values(key_with_values));
	else if (key == "client_max_body_size")
		parse_client_max_body_size(size, server, key_with_values);
	else if (key == "autoindex" && size >= 2 && key_with_values[1] == "true")
		server.setAutoindex(true);
	else if (key == "index" && size >= 2)
		server.setIndexFile(key_with_values[1]);
	else if (key == "root" && size >= 2)
		server.setRoot(key_with_values[1]);
	else if (key == "deny" || key == "allow")
		set_allowed_methods(server, key_with_values);
	else if (key == "cgi_path" && size >= 2)
		server.setCgiPath(key_with_values[1]);
	else if (key == "cgi_extension" && size >= 2)
		server.setCgiExtension(key_with_values[1]);
}

void set_server_config_settings(Server &server, std::string str, int i) {

	i = iterate_to_first_server_line(str, i);
	while (str[i] && str[i] != '}') {
		if (str.substr(i, 8) == "location")
			i = parse_route(server, str, i);
		else
			parse_key_with_values(server, str, i);
		i = iterate_to_next_server_line(str, i);
	}
	if (str[i] != '}')
		throw std::runtime_error("Ending curly brace missing: '}'");
}

Config::Config(const std::string filename) {
	if (filename == "")
		throw std::runtime_error("No filename provided");
	std::string file_content = get_file_content(filename);

	size_t index = file_content.find("server");
	while (index != std::string::npos) {
		Server server_obj;

		set_server_config_settings(server_obj, file_content, index + 6);
		server_obj.setup();
		print_server_obj(server_obj);
		this->_servers.push_back(server_obj);
		while (file_content[index] && file_content[index] != '}') {
			index++;
		}
		index = file_content.find("server", index);
	}
}

std::vector<Server>& Config::get_servers() { return (this->_servers); }