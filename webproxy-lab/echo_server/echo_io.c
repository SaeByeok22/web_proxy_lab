#include "echo.h"

// rio_t와 fd를 연결해서 내부 버퍼를 처음 상태로 만든다.
void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

// 내부 버퍼에서 최대 n바이트를 꺼낸다. 비어 있으면 fd에서 다시 읽는다.
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    size_t cnt;

    while (rp->rio_cnt <= 0) {
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0) {
            if (errno != EINTR) {
                return -1;
            }
        } else if (rp->rio_cnt == 0) {
            return 0;
        } else {
            rp->rio_bufptr = rp->rio_buf;
        }
    }

    cnt = n;
    if ((size_t)rp->rio_cnt < n) {
        cnt = (size_t)rp->rio_cnt;
    }

    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= (int)cnt;
    return (ssize_t)cnt;
}

// fd에서 한 줄을 읽어서 usrbuf에 저장한다.
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    char c;
    char *bufp = usrbuf;
    size_t n;

    for (n = 1; n < maxlen; n++) {
        ssize_t rc = rio_read(rp, &c, 1);
        if (rc == 1) {
            *bufp++ = c;
            if (c == '\n') {
                n++;
                break;
            }
        } else if (rc == 0) {
            if (n == 1) {
                return 0;
            }
            break;
        } else {
            return -1;
        }
    }

    *bufp = '\0';
    return (ssize_t)(n - 1);
}

// write가 중간에 덜 쓰는 경우까지 처리해서 n바이트를 끝까지 보낸다.
ssize_t rio_writen(int fd, const void *usrbuf, size_t n)
{
    const char *bufp = usrbuf;
    size_t nleft = n;

    while (nleft > 0) {
        ssize_t nwritten = write(fd, bufp, nleft);
        if (nwritten <= 0) {
            if (nwritten < 0 && errno == EINTR) {
                nwritten = 0;
            } else {
                return -1;
            }
        }

        nleft -= (size_t)nwritten;
        bufp += nwritten;
    }

    return (ssize_t)n;
}
