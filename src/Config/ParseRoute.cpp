#include <stdexcept>
#include <stdlib.h>
#include "Config.hpp"


void set_allowed_methods(Route &route, std::vector<std::string> key_with_values) {
	bool new_state;

	if (key_with_values[0] == "allow")
		new_state = true;
	else if (key_with_values[0] == "deny")
		new_state = false;
	else
		return;

	size_t i = 1;
	while (i < key_with_values.size()) {
		std::string key = key_with_values[i];
		if (key == "all") {
			if (route.POST.can_be_edited)
				route.POST.is_allowed = new_state;
			if (route.GET.can_be_edited)
				route.GET.is_allowed = new_state;
			if (route.DELETE.can_be_edited)
				route.DELETE.is_allowed = new_state;
		} else if (key == "POST" && route.POST.can_be_edited) {
			route.POST.is_allowed = new_state;
			route.POST.can_be_edited = false;
		} else if (key == "GET" && route.GET.can_be_edited) {
			route.GET.is_allowed = new_state;
			route.GET.can_be_edited = false;
		} else if (key == "DELETE" && route.DELETE.can_be_edited) {
			route.DELETE.is_allowed = new_state;
			route.DELETE.can_be_edited = false;
		}
		i++;
	}
}

void parse_location(int value_count, std::vector<std::string> &values, Route &route) {
	if (value_count < 2 || value_count > 3 || values[value_count - 1] != "{")
		throw std::runtime_error("Bad route format on first line.");
	if (value_count == 3)
		route.matching_style = values[0];
	route.location = values[value_count - 2];
	if (route.location[route.location.size() - 1] != '/')
		route.location += '/';
}

void parse_redirection(std::vector<std::string> &values, int value_count, Route &route) {
	if (isNumeric(values[0])) {
		route.redirection.code = atoi(values[0].c_str());
		if (value_count == 2)
			route.redirection.url = values[value_count - 1];
	}
}

int parse_route(Server &virtual_server, std::string str, int i) {
	Route route;
	std::vector<std::string> key_with_values;
	std::string key;
	std::vector<std::string> values;
	int value_count;

	// i = iterate_to_first_server_line(str, i);
	while (str[i] && str[i] != '}') {
		key_with_values = convert_server_line_2_vector(str, i);
		key = key_with_values[0];
		values = extract_values(key_with_values);
		value_count = values.size();

		if (key == "location")
			parse_location(value_count, values, route);
		else if (key == "root" && value_count == 1)
			route.root = values[0];
		else if (key == "return" && value_count >= 1)
			parse_redirection(values, value_count, route);
		else if (key == "autoindex" && value_count == 1 && toLowerCase(values[0]) == "true")
			route.autoindex = true;
		else if (key == "index" && value_count >= 1)
			route.index_file = values[0];
		else if (key == "deny" || key == "allow")
			set_allowed_methods(route, key_with_values);
		else if (key == "cgi_path" && value_count >= 1)
			route.cgi_path = key_with_values[1];
		else if (key == "cgi_extension" && value_count >= 1)
			route.cgi_extension = key_with_values[1];
		i = iterate_to_next_server_line(str, i);
	}
	if (str[i] == '}')
		i++;

	virtual_server.addRoute(route);
	return (i);
}
