#include "echo.h"

// port에서 접속을 기다리는 listening socket을 만든다.
int open_listenfd(const char *port)
{
    int listenfd = -1;
    int optval = 1;
    struct addrinfo hints;
    struct addrinfo *listp;
    struct addrinfo *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    #ifdef AI_ADDRCONFIG

    hints.ai_flags |= AI_ADDRCONFIG;

    #endif

    if (getaddrinfo(NULL, port, &hints, &listp) != 0) {
        return -1;
    }

    for (p = listp; p != NULL; p = p->ai_next) {
        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listenfd < 0) {
            continue;
        }

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(optval));

        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }

        close(listenfd);
        listenfd = -1;
    }

    freeaddrinfo(listp);

    if (listenfd < 0) {
        return -1;
    }

    if (listen(listenfd, LISTENQ) < 0) {
        close(listenfd);
        return -1;
    }

    return listenfd;
}
