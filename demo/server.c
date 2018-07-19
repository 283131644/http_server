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

    // 理解一下 htonl 和 htons 是干什么的
    // INADDR_ANY 表示什么？
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
        // 如何获取客户端 IP:PORT
        printf("Client connected: %d\n", clientfd);

        // 理解一下fork
        ret = fork();
        if (ret < 0) {
            perror("fork");
            exit(-1);
        }
        if (ret == 0) {
            handle_client(root_path, clientfd);
            // 如果缺少这个exit，会不会有问题？
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
    // 为什么需要一个循环来读取
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

    // 获取 URI
    strtok(req, " ");
    char *uri = strtok(NULL, " ");

    printf("[%d] request_uri=%s\n", fd, uri);

    // 这里只是给个Demo，直接响应"Hello World";
    // 按要求替换成相应的文件内容，文件路径为<root_path>/<uri>
    // 考察一下怎么防攻击：比如读到其它目录下的文件
    // 如果文件没有找到，应该怎么办？
    char *str = "HTTP/1.1 200 OK\r\n"
        "Content-Length: 12\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Hello, World";
    write(fd, str, strlen(str));
    close(fd);
}
