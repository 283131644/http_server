#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAX_REQ 1024

void handle_client(char *root_path, int fd);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <root-path> <port>\n", argv[0]);
        exit(-1);
    }
    char *root_path = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;

    // What does the `htonl` and `htons` mean?
    // What is `INADDR_ANY`?
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket");
        exit(-1);
    }

    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(-1);
    }

    if (listen(listenfd, 5) < 0) {
        perror("listen");
        exit(-1);
    }

    int clientfd = -1;
    int ret = 0;
    while ((clientfd = accept(listenfd, NULL, NULL)) >= 0) {
        // How to print the client's ip and port?
        printf("Client connected: %d\n", clientfd);

        // Understand `fork`
        ret = fork();
        if (ret < 0) {
            perror("fork");
            exit(-1);
        }
        if (ret == 0) {
            handle_client(root_path, clientfd);
            // If this `exit` miss, what will happend?
            exit(0);
        }
    }

    return 0;
}

void handle_client(char *root_path, int fd) {
    char req[MAX_REQ];
    int req_len = 0;
    req[req_len] = '\0';

    int n = 0;
    // Why we need a loop here?
    while (strstr(req, "\r\n\r\n") == NULL) {
        n = read(fd, req + req_len, MAX_REQ - req_len);
        if (n < 0) {
            perror("read");
            exit(-1);
        }
        if (n == 0) {
            fprintf(stderr, "client closed");
            return;
        }
        req_len += n;
        req[req_len] = '\0';
    }

    // Get the URI
    strtok(req, " ");
    char *uri = strtok(NULL, " ");

    printf("[%d] request_uri=%s\n", fd, uri);

    // Replace it to read the file in the dir root_path base on the uri
    // Response "Hello World";
    char *str = "HTTP/1.1 200 OK\r\n"
        "Content-Length: 12\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Hello, World";
    write(fd, str, strlen(str));
    close(fd);
}
