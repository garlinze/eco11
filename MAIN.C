#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

static const char *response_template =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n\r\n"
    "<!DOCTYPE html>\n"
    "<html><head><title>Hello , wol!</title></head>\n"
    "<body><h1>Hello, World!</h1></body></html>\r\n";

int create_socket(void) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void configure_socket(int sockfd, int port) {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
}

void listen_on_socket(int sockfd, int backlog) {
    if (listen(sockfd, backlog) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

ssize_t read_from_socket(int sockfd, void *buf, size_t count) {
    ssize_t n;
    if ((n = read(sockfd, buf, count)) == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    return n;
}

void write_to_socket(int sockfd, const void *buf, size_t count) {
    if (write(sockfd, buf, count) == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    int sockfd = create_socket();
    configure_socket(sockfd, PORT);
    listen_on_socket(sockfd, 3);

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sockfd == -1) {
            perror("accept");
            continue;
        }

        char buffer[BUFFER_SIZE];
        ssize_t n = read_from_socket(client_sockfd, buffer, BUFFER_SIZE);
        fprintf(stderr, "Received %ld bytes:\n%s\n", (long)n, buffer);

        write_to_socket(client_sockfd, response_template, strlen(response_template));
        close(client_sockfd);
    }

    close(sockfd);
    return 0;
}