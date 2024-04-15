#include "Config.hpp"
#define GREEN "\033[32m"
#define RESET "\033[0m"

std::string get_file_content(const std::string &filename) {
	std::ifstream file(filename.c_str());
	std::string line;
	std::string file_content;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            file_content += line + '\n';
        }
        file.close();
    } else {
        std::cerr << "Failed to open the file." << std::endl;
		// throw error????
    }
	file_content += '\0';
	return (file_content);
}

bool is_whitespace_char(char c) {
	if (c == ' ' || c == '\t' || c == '\n')
		return (true);
	else
		return (false);
}

template <typename T>
void    print_vector(T vector) {
    for (size_t i = 0; i < vector.size(); ++i) {
        std::cout << vector[i] << " ";
    }
    std::cout << std::endl;
}

void	print_server_routes(std::vector<Route> routes) {
    for (size_t i = 0; i < routes.size(); ++i) {
        std::cout << "\tRoute No." << i + 1 << std::endl;
        if (routes[i].location != "")
            std::cout << "\t\tLocation: " << routes[i].location << std::endl;
        if (routes[i].matching_style != "")
            std::cout << "\t\tMatching: " << routes[i].matching_style << std::endl;
        if (routes[i].root != "")
            std::cout << "\t\tRoot: " << routes[i].root << std::endl;
        for (size_t j = 0; j < routes[i].redirections.size(); ++j) {
            std::cout << "\t\tRedirection No." << j + 1 << std::endl;
            std::cout << "\t\t\tCode: " << routes[i].redirections[j].code << std::endl;
            std::cout << "\t\t\tUrl: " << routes[i].redirections[j].url << std::endl;
        }
    }
    std::cout << std::endl;
}

void	print_server_obj(Virtual_Server_Config obj)
{
	std::cout << GREEN << "SERVER_OBJ" << RESET << std::endl;
    if (obj.port)
        std::cout << "\tPort: " << obj.port << std::endl;
    if (obj.server_names.size() > 0) {
        std::cout << "\tServer names: ";
        print_vector(obj.server_names);
    }
    if (obj.error_pages.size() > 0) {
        std::cout << "\tError pages: ";
        print_vector(obj.error_pages);
    }
    if (obj.client_max_body_size.size() > 0)
        std::cout << "\tClientMaxBodySize " << obj.client_max_body_size << std::endl;
    if (obj.routes.size() > 0)
        print_server_routes(obj.routes);
    std::cout << std::endl;
}


