#include "Config.hpp"

void    set_allowed_methods(Route &route, std::vector<std::string> key_with_values) {
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
            if (route.POST.can_be_edited)
                route.POST.is_allowed = new_state;
            if (route.GET.can_be_edited)
                route.GET.is_allowed = new_state;
        }
        else if (key == "POST" && route.POST.can_be_edited) {
            route.POST.is_allowed = new_state;
            route.POST.can_be_edited = false;
        }
        else if (key == "GET" && route.GET.can_be_edited) {
            route.GET.is_allowed = new_state;
            route.GET.can_be_edited = false;
        }
        i++;
    }
}

void parse_location(int value_count, std::vector<std::string> &values, Route &route)
{
	if (value_count < 2 || value_count > 3 || values[value_count - 1] != "{")
		throw std::runtime_error("Bad route format on first line.");
	if (value_count == 3)
		route.matching_style = values[0];
	route.location = values[value_count - 2];
}

void parse_redirection(std::vector<std::string> &values, int value_count, Route &route)
{
	HttpRedirection redirection(values[value_count - 1]);
	if (value_count == 2)
		redirection.code = values[0];
	route.redirections.push_back(redirection);
}

void parse_param(std::vector<std::string> &values, Route &route)
{
	Fastcgi_Param param(values[0], values[1]);
	route.fastcgi_params.push_back(param);
}

int	parse_route(Server &virtual_server, std::string str, int i) {
	Route						route;
	std::vector<std::string> 	key_with_values;
    std::string                 key;
	std::vector<std::string> 	values;
	int							value_count;

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
		else if (key == "return" && value_count >= 1 && value_count <= 2)
			parse_redirection(values, value_count, route);
		else if (key == "autoindex" && value_count == 1 && toLowerCase(values[0]) == "true")
			route.autoindex = true;
		else if (key == "index" && value_count >= 1)
			route.index_files = values;
		else if (key == "deny" || key == "allow")
            set_allowed_methods(route, key_with_values);
		else if (key == "fastcgi_pass" && value_count == 1)
			route.fastcgi_pass = values[0];
		else if (key == "fastcgi_index" && value_count == 1)
			route.fastcgi_index = values[0];
		else if (key == "fastcgi_param" && value_count == 2)
			parse_param(key_with_values, route);

		i = iterate_to_next_server_line(str, i);
	}
	if (str[i] == '}')
		i++;
	virtual_server._routes.push_back(route);
	return (i);
}


