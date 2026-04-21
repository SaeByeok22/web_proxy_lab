#include "csapp.h"
#include <strings.h>

static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void doit(int connfd);
int parse_uri(char *uri, char *host, char *port, char *path);
void build_request_header(char *http_header, char *host, char *path,
                          rio_t *client_rio);
void relay_response(int serverfd, int connfd);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv)
{
  int listenfd, connfd;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  Signal(SIGPIPE, SIG_IGN);
  listenfd = Open_listenfd(argv[1]);

  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    doit(connfd);
    Close(connfd);
  }
}

/* 클라이언트 요청 하나를 순차적으로 처리한다. */
void doit(int connfd)
{
  int serverfd;
  rio_t client_rio;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char host[MAXLINE], port[16], path[MAXLINE];
  char http_header[MAXBUF];

  Rio_readinitb(&client_rio, connfd);
  if (Rio_readlineb(&client_rio, buf, MAXLINE) <= 0)
    return;

  if (sscanf(buf, "%s %s %s", method, uri, version) != 3)
  {
    clienterror(connfd, buf, "400", "Bad Request",
                "Proxy could not parse the request line");
    return;
  }

  if (strcasecmp(method, "GET"))
  {
    clienterror(connfd, method, "501", "Not Implemented",
                "Proxy only implements GET");
    return;
  }

  if (!parse_uri(uri, host, port, path))
  {
    clienterror(connfd, uri, "400", "Bad Request",
                "Proxy could not parse the URI");
    return;
  }

  build_request_header(http_header, host, path, &client_rio);

  serverfd = open_clientfd(host, port);
  if (serverfd < 0)
  {
    clienterror(connfd, host, "502", "Bad Gateway",
                "Proxy failed to connect to the end server");
    return;
  }

  Rio_writen(serverfd, http_header, strlen(http_header));
  relay_response(serverfd, connfd);
  Close(serverfd);
}

/* absolute URI에서 host, port, path를 분리한다. */
int parse_uri(char *uri, char *host, char *port, char *path)
{
  char *hostbegin;
  char *pathbegin;
  char *portbegin;
  char hostbuf[MAXLINE];
  size_t hostlen;

  hostbegin = uri;
  if (!strncasecmp(uri, "http://", 7))
    hostbegin = uri + 7;

  pathbegin = strchr(hostbegin, '/');
  if (pathbegin)
  {
    snprintf(path, MAXLINE, "%s", pathbegin);
    hostlen = (size_t)(pathbegin - hostbegin);
    if (hostlen >= sizeof(hostbuf))
      hostlen = sizeof(hostbuf) - 1;
    memcpy(hostbuf, hostbegin, hostlen);
    hostbuf[hostlen] = '\0';
  }
  else
  {
    strcpy(path, "/");
    snprintf(hostbuf, sizeof(hostbuf), "%s", hostbegin);
  }

  portbegin = strchr(hostbuf, ':');
  if (portbegin)
  {
    *portbegin = '\0';
    portbegin++;
    snprintf(port, 16, "%s", portbegin);
  }
  else
  {
    strcpy(port, "80");
  }

  if (strlen(hostbuf) == 0)
    return 0;

  snprintf(host, MAXLINE, "%s", hostbuf);
  return 1;
}

/* 원격 서버로 보낼 HTTP/1.0 요청 헤더를 다시 만든다. */
void build_request_header(char *http_header, char *host, char *path,
                          rio_t *client_rio)
{
  char buf[MAXLINE];
  char request_line[MAXLINE];
  char host_hdr[MAXLINE];
  char other_hdr[MAXBUF];
  int has_host = 0;

  snprintf(request_line, sizeof(request_line), "GET %s HTTP/1.0\r\n", path);
  other_hdr[0] = '\0';
  host_hdr[0] = '\0';

  while (Rio_readlineb(client_rio, buf, MAXLINE) > 0)
  {
    if (!strcmp(buf, "\r\n"))
      break;

    if (!strncasecmp(buf, "Host:", 5))
    {
      has_host = 1;
      snprintf(host_hdr, sizeof(host_hdr), "%s", buf);
      continue;
    }

    if (!strncasecmp(buf, "User-Agent:", 11) ||
        !strncasecmp(buf, "Connection:", 11) ||
        !strncasecmp(buf, "Proxy-Connection:", 17))
    {
      continue;
    }

    strncat(other_hdr, buf, sizeof(other_hdr) - strlen(other_hdr) - 1);
  }

  if (!has_host)
    snprintf(host_hdr, sizeof(host_hdr), "Host: %s\r\n", host);

  snprintf(http_header, MAXBUF,
           "%s%s%sConnection: close\r\nProxy-Connection: close\r\n%s\r\n",
           request_line, host_hdr, user_agent_hdr, other_hdr);
}

/* 원격 서버의 응답을 끝까지 읽어서 클라이언트에게 전달한다. */
void relay_response(int serverfd, int connfd)
{
  ssize_t n;
  char buf[MAXBUF];
  rio_t server_rio;

  Rio_readinitb(&server_rio, serverfd);
  while ((n = Rio_readnb(&server_rio, buf, MAXBUF)) > 0)
  {
    Rio_writen(connfd, buf, n);
  }
}

/* 간단한 HTTP 에러 응답을 보낸다. */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  snprintf(body, sizeof(body),
           "<html><title>Proxy Error</title>"
           "<body bgcolor=\"ffffff\">\r\n"
           "%s: %s\r\n"
           "<p>%s: %s\r\n"
           "<hr><em>The Proxy server</em>\r\n",
           errnum, shortmsg, longmsg, cause);

  snprintf(buf, sizeof(buf), "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  snprintf(buf, sizeof(buf), "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  snprintf(buf, sizeof(buf), "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}
