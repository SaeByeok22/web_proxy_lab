#include "echo.h"

// 서버 echo: 클라이언트가 보낸 줄을 읽고 그대로 다시 보낸다.
static void echo(int connfd)
{
    rio_t rio;
    char line[MAXLINE];
    ssize_t nread;

    rio_readinitb(&rio, connfd);

    while ((nread = rio_readlineb(&rio, line, sizeof(line))) > 0) {
        if (rio_writen(connfd, line, (size_t)nread) < 0) {
            break;
        }
    }
}

// 서버 main: port를 열고 클라이언트를 하나씩 받아 echo를 수행한다.
int main(int argc, char **argv)
{
    int listenfd;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }

    listenfd = open_listenfd(argv[1]);
    if (listenfd < 0) {
        fprintf(stderr, "failed to listen on port %s\n", argv[1]);
        return 1;
    }

    for (;;) {
        struct sockaddr_storage clientaddr;
        socklen_t clientlen = sizeof(clientaddr);
        int connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);

        if (connfd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            continue;
        }

        echo(connfd);
        close(connfd);
    }
}
