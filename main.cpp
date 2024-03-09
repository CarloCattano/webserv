#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

const int MAX_EVENTS = 10;
const int BACKLOG = 10;
const int BUFFER_SIZE = 1024;

void handle_request(int client_fd) {
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, World!";
    send(client_fd, response.c_str(), response.size(), 0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [configuration file]" << std::endl;
        return 1;
    }

    // Parse configuration file if needed

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
    server_addr.sin_port = htons(8080); // Change port as needed

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

