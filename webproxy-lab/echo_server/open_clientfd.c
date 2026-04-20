#include "echo.h"

// hostname:port로 접속하는 클라이언트 소켓을 만든다.
int open_clientfd(const char *hostname, const char *port)
{
    int clientfd = -1;
    struct addrinfo hints;
    struct addrinfo *listp;
    struct addrinfo *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    
    #ifdef AI_ADDRCONFIG

    hints.ai_flags |= AI_ADDRCONFIG;

    #endif

    if (getaddrinfo(hostname, port, &hints, &listp) != 0) {
        return -1;
    }

    for (p = listp; p != NULL; p = p->ai_next) {
        clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (clientfd < 0) {
            continue;
        }

        if (connect(clientfd, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }

        close(clientfd);
        clientfd = -1;
    }

    freeaddrinfo(listp);
    return clientfd;
}
