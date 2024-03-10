/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ccattano <ccattano@42Berlin.de>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/10 11:19:25 by ccattano          #+#    #+#             */
/*   Updated: 2024/03/10 11:44:10 by ccattano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map> // Include this for unordered_map

const int MAX_EVENTS = 10;
const int BACKLOG = 10;
const int BUFFER_SIZE = 1024;

int PORT = 8080;   // TODO needs to be set by config file


std::map<std::string, std::string> content_types;

// Function to populate content_types map
void populateContentTypes() {
    content_types[".html"] = "text/html";
    content_types[".css"] = "text/css";
    content_types[".jpg"] = "image/jpeg";
    content_types[".jpeg"] = "image/jpeg";
    content_types[".png"] = "image/png";
    // Add more file extensions and corresponding content types as needed
}

std::string getContentType(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string extension = filename.substr(dotPos);
        if (content_types.find(extension) != content_types.end()) {
            return content_types[extension];
        }
    }
    // Default to plain text if content type not found
    return "text/plain";
}

/* print info to the terminal with all the request data */
/* void debug_request(const char *buffer, int size) { */
/*     std::cout << "---------------------\n"; */
/*     std::string request(buffer, size); */
/*     std::cout << "Received request:" << "\n"; */
/*     std::cout << request << "\n"; */
/*     std::cout << "---------------------" << std::endl; */
/* } */

std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string intToString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string extract_requested_file_path(const char *buffer) {
    std::string request(buffer);
    size_t start = request.find("GET") + 4;
    size_t end = request.find("HTTP/1.1") - 1;
    std::string path = request.substr(start, end - start);
    return path;
}

void handle_request(int client_fd) {
    char buffer[BUFFER_SIZE];
    int size = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (size == -1) {
        perror("recv");
        return;
    }

    /* debug_request(buffer, size); */

    std::string requested_file_path = extract_requested_file_path(buffer);
    std::string file_content = readFileToString("website" + requested_file_path);

    if (file_content.empty()) {
        // File not found or error reading file
        std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
    } else {
        // Determine content type based on file extension
        std::string content_type = getContentType(requested_file_path);

        // Construct HTTP response
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type + "\r\nContent-Length: " +
                               intToString(file_content.length()) + "\r\n\r\n" + file_content;

        /* std::cout << response << std::endl; */
        send(client_fd, response.c_str(), response.size(), 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [configuration file]" << std::endl;
        return 1;
    }

    // Parse configuration here

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }
    
    populateContentTypes();

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, BACKLOG) == -1) {
        perror("listen");
        return 1;
    }

    struct pollfd fds[MAX_EVENTS];
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    std::cout << "Server listening on http://localhost:" << PORT << std::endl;

    while (true) 
    {
        int activity = poll(fds, MAX_EVENTS, -1);
        if (activity == -1) {
            perror("poll");
            continue;
        }

        if (fds[0].revents & POLLIN) {
            int client_fd = accept(server_fd, NULL, NULL);
            if (client_fd == -1) {
                perror("accept");
                continue;
            }

            handle_request(client_fd); 
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}
