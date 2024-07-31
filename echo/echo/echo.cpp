#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

constexpr int MAX_EVENTS = 10;
constexpr int BUFFER_SIZE = 1024;
constexpr int PORT = 2971;

void set_non_blocking(int fd) {
    int opts = fcntl(fd, F_GETFL);    // Get the current file descriptor flags
    if (opts < 0) {
        perror("fcntl(F_GETFL) failed");
        close(fd);
        exit(EXIT_FAILURE);
    }
    opts |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, opts) < 0) {
        perror("fcntl(F_SETFL) failed");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

void epoll_add(int fd, int epoll_fd, epoll_event ev) {
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl(EPOLL_CTL_ADD)");
        close(fd);
        exit(EXIT_FAILURE);
    }
}
 

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // creating an endpoint for our server
    if (server_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    set_non_blocking(server_fd);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT); // creating an address for the socket
    int addrlen = sizeof(address);

    if (bind(server_fd, (struct sockaddr*)&address, addrlen) < 0) { // bind the address to our socket
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) { // allow the server socket to listen to up to 3 requests
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    int epoll_fd = epoll_create1(0); // creating an epoll instance
    if (epoll_fd == -1) {
        perror("epoll_create1 failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev, events[MAX_EVENTS];
    epoll_add(server_fd, epoll_fd, ev);
    char buffer[BUFFER_SIZE];
    while (true) {
        int cnt = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); // wait until epoll picks up a client
        for (int i = 0; i < cnt; ++i) { // process all of the client's events
            if (events[i].data.fd == server_fd) { // there is a new connection on the server socket
                while (true) {
                    int client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen); // accept connection
                    if (client_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) { // no incoming connections
                            break; 
                        } else {
                            perror("accept failed");
                            exit(EXIT_FAILURE);
                        }
                    } else { // accepeted the connection successfully
                        set_non_blocking(client_fd); // build client socket and add to epoll
                        epoll_add(client_fd, epoll_fd, ev);
                    }
                }
            } else { // a client socket has recieved data
                int client_fd = events[i].data.fd;
                int bytes_read = read(client_fd, buffer, BUFFER_SIZE);
                if (bytes_read == -1) {
                    perror("read failed");
                    close(client_fd);
                } else if (bytes_read == 0) {
                    close(client_fd);
                } else {
                    if (write(client_fd, buffer, bytes_read) == -1) {
                        perror("write failed");
                        close(client_fd);
                    }
                }
            }
        }
    }
    close(server_fd);
    return 0;
}
