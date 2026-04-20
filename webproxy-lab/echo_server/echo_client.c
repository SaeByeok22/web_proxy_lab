#include "echo.h"

// 클라이언트 main: 서버에 접속하고, 입력한 줄을 서버로 보낸 뒤 응답을 출력한다.
int main(int argc, char **argv)
{
    int clientfd;
    rio_t rio;
    char sendline[MAXLINE];
    char recvline[MAXLINE];

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        return 1;
    }

    clientfd = open_clientfd(argv[1], argv[2]);
    if (clientfd < 0) {
        fprintf(stderr, "failed to connect to %s:%s\n", argv[1], argv[2]);
        return 1;
    }

    rio_readinitb(&rio, clientfd);

    while (fgets(sendline, sizeof(sendline), stdin) != NULL) {
        size_t len = strlen(sendline);
        if (rio_writen(clientfd, sendline, len) < 0) {
            perror("write");
            break;
        }

        if (rio_readlineb(&rio, recvline, sizeof(recvline)) <= 0) {
            break;
        }

        fputs(recvline, stdout);
    }

    close(clientfd);
    return 0;
}
