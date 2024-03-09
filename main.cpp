#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>

const int MAX_EVENTS = 10;
const int BACKLOG = 10;
const int BUFFER_SIZE = 1024;

const int PORT = 8080;

/* print info to the terminal with all the request data */
void debug_request(const char *buffer, int size) {
    std::string request(buffer, size);
    std::cout << "Received request:" << std::endl;
    std::cout << request << std::endl;
}

// TODO replace Hello, World with the website/index.html
void handle_request(int client_fd) {

    char buffer[BUFFER_SIZE];
    int size = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (size == -1) {
        perror("recv");
        return;
    }

    debug_request(buffer, size);

    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 68\r\n\r\n<html><head>Hello, World!</head><body><p>hello</p></body></html>\r\n\r\n"; 
    send(client_fd, response.c_str(), response.size(), 0);
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

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT); // Change port as needed

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

    while (true) {
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

