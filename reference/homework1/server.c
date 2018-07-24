/**
 * Homework1：实现一个简单版的 WebServer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>

#define MAX_BUF 1024
#define MAX_PATH 128

// 了解一下 C语言中这种字符串的写法
char *str_404 = "HTTP/1.1 404 Not Found\r\n"
                "Content-Length: 20\r\n"
                "Content-Type: text/html\r\n"
                "\r\n"
                "<p>404 Not Found<p/>";

void handle_client(char *root_path, int fd);


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <root-path> <port>\n", argv[0]);
        exit(-1);
    }
    // 读取参数
    char *root_path = argv[1];
    int port = atoi(argv[2]);

    // 初始化地址
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;

    // 理解一下 htonl 和 htons 是干什么的
    // INADDR_ANY 表示什么？
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    // 创建socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket");
        exit(-1);
    }

    // 这个是干什么的？
    int val = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // 绑定地址
    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(-1);
    }

    // 监听
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
            close(listenfd);
            handle_client(root_path, clientfd);
            // 如果缺少这个exit，会不会有问题？
            exit(0);
        }

        // 如果漏掉这个，会导致什么问题？
        close(clientfd);
    }

    return 0;
}

int str_endswith(char *str, char *end) {
    int len1 = strlen(str);
    int len2 = strlen(end);
    if (len2 > len1) {
        return 0;
    }
    return strcmp(str + len1 - len2, end) == 0;
}

void handle_client(char *root_path, int fd) {
    char req[MAX_BUF];
    int req_len = 0;
    req[req_len] = '\0';

    int n = 0;
    // 为什么需要一个循环来读取
    while (strstr(req, "\r\n\r\n") == NULL) {
        n = read(fd, req + req_len, MAX_BUF - req_len);
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

    if (strcmp(uri, "/") == 0) {
        // 默认文件
        uri = "/index.html";
    }

    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "%s%s", root_path, uri);
    printf("%s\n", file_path);

    // 根据文件名后缀得到 MIME Type: MIME Type 有什么用？
    char *type = "text/plain";
    if (str_endswith(uri, ".html")) {
        type = "text/html";
    } else if (str_endswith(uri, ".js")) {
        type = "text/javascript";
    } else if (str_endswith(uri, ".css")) {
        type = "text/css";
    } else if (str_endswith(uri, ".jpg")) {
        type = "image/jpeg";
    }

    // stat 有什么作用？什么情况下会出错？
    struct stat st;
    if (stat(file_path, &st) < 0) {
        write(fd, str_404, strlen(str_404));
        close(fd);
        return;
    }

    // 这里有什么Bug?
    char *content = malloc(st.st_size);
    int fileno = open(file_path, O_RDONLY);
    if (fileno < 0) {
        perror("open");
        close(fd);
        return;
    }
    read(fileno, content, st.st_size);
    close(fileno);

    char head[MAX_BUF];
    snprintf(head, sizeof(head), "HTTP/1.1 200 OK\r\n"
                                 "Content-Length: %lu\r\n"
                                 "Content-Type: %s\r\n\r\n", st.st_size, type);
    write(fd, head, strlen(head));
    write(fd, content, st.st_size);
    close(fd);
}
