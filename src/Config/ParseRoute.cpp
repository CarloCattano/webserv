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
		else if (key_with_values[0] == "index" && size >= 2)
			route.index_files = get_values(key_with_values);
		else if (key_with_values[0] == "deny" || key_with_values[0] == "allow")
            set_allowed_methods(route, key_with_values);
		i = iterate_to_next_server_line(str, i);
	}
	if (str[i] == '}')
		i++;
	virtual_server.routes.push_back(route);
	return (i);
}
