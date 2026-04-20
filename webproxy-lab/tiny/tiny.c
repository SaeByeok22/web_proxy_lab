/*
 * tiny.c - A small iterative HTTP/1.0 web server.
 */
#include "csapp.h"
#include <strings.h>

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize, int head_only);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs, int head_only);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg, int head_only);

// 지정한 포트에서 클라이언트 연결을 계속 받고, 연결마다 doit()으로 HTTP 요청을 처리한다.
int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  printf("[debug] tiny server start, port=%s\n", argv[1]);

  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    printf("[debug] client connected: %s:%s\n", hostname, port);
    doit(connfd);
    Close(connfd);
  }
}

// HTTP 요청 하나를 읽고, 메소드와 URI에 따라 정적 파일 응답 또는 CGI 응답을 선택해서 보낸다.
void doit(int fd)
{
  int is_static, head_only;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio, fd);
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
    return;

  printf("%s", buf);
  printf("[debug] request line: %s", buf);

  if (sscanf(buf, "%s %s %s", method, uri, version) != 3)
  {
    clienterror(fd, buf, "400", "Bad Request",
                "Tiny could not parse the request line", 0);
    return;
  }

  printf("[debug] method=%s, uri=%s, version=%s\n", method, uri, version);

  head_only = !strcasecmp(method, "HEAD");
  if (strcasecmp(method, "GET") && strcasecmp(method, "HEAD"))
  {
    clienterror(fd, method, "501", "Not Implemented",
                "Tiny only implements GET and HEAD", 0);
    return;
  }

  read_requesthdrs(&rio);

  is_static = parse_uri(uri, filename, cgiargs);
  printf("[debug] file=%s, cgiargs=%s, static=%d, head=%d\n",
         filename, cgiargs, is_static, head_only);

  if (stat(filename, &sbuf) < 0)
  {
    clienterror(fd, filename, "404", "Not Found",
                "Tiny could not find this file", head_only);
    return;
  }

  if (is_static)
  {
    if (!S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny could not read this file", head_only);
      return;
    }

    serve_static(fd, filename, sbuf.st_size, head_only);
  }
  else
  {
    if (!S_ISREG(sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny could not run this CGI program", head_only);
      return;
    }

    serve_dynamic(fd, filename, cgiargs, head_only);
  }
}

// 요청 헤더를 빈 줄까지 읽고 버린다. 출력은 디버깅용 로그뿐이고, 응답 body는 만들지 않는다.
void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];
  int header_count = 0;

  while (Rio_readlineb(rp, buf, MAXLINE) > 0)
  {
    printf("%s", buf);
    if (!strcmp(buf, "\r\n"))
      break;
    header_count++;
  }

  printf("[debug] header count=%d\n", header_count);
}

// URI를 실제 파일 경로로 바꾸고, 정적 요청이면 1, CGI 요청이면 0을 반환한다.
int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr;

  if (!strstr(uri, "cgi-bin"))
  {
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    if (uri[strlen(uri) - 1] == '/')
      strcat(filename, "home.html");
    printf("[debug] static uri maps to %s\n", filename);
    return 1;
  }

  ptr = strchr(uri, '?');
  if (ptr)
  {
    strcpy(cgiargs, ptr + 1);
    *ptr = '\0';
  }
  else
  {
    strcpy(cgiargs, "");
  }

  strcpy(filename, ".");
  strcat(filename, uri);
  printf("[debug] dynamic uri maps to %s, args=%s\n", filename, cgiargs);
  return 0;
}

// 정적 파일의 HTTP 200 응답을 만든다. GET이면 malloc으로 파일을 읽어 body를 보내고, HEAD면 헤더만 보낸다.
void serve_static(int fd, char *filename, int filesize, int head_only)
{
  int srcfd;
  char *srcp = NULL;
  char filetype[64], buf[MAXBUF];

  if (!head_only && filesize > 0)
  {
    srcp = malloc(filesize);
    if (srcp == NULL)
    {
      clienterror(fd, filename, "500", "Internal Server Error",
                  "Tiny could not allocate memory for this file", 0);
      return;
    }

    srcfd = Open(filename, O_RDONLY, 0);
    if (rio_readn(srcfd, srcp, filesize) != filesize)
    {
      Close(srcfd);
      free(srcp);
      clienterror(fd, filename, "500", "Internal Server Error",
                  "Tiny could not read this file", 0);
      return;
    }
    Close(srcfd);
  }

  get_filetype(filename, filetype);
  printf("[debug] static response: %s, %d bytes, %s\n",
         filename, filesize, filetype);

  snprintf(buf, sizeof(buf),
           "HTTP/1.0 200 OK\r\n"
           "Server: Tiny Web Server\r\n"
           "Connection: close\r\n"
           "Content-length: %d\r\n"
           "Content-type: %s\r\n\r\n",
           filesize, filetype);

  rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n%s", buf);

  if (head_only)
    return;

  if (filesize > 0)
  {
    rio_writen(fd, srcp, filesize);
    free(srcp);
  }
}

// 파일 확장자를 보고 HTTP 응답의 Content-type 값을 정한다.
void get_filetype(char *filename, char *filetype)
{
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg") || strstr(filename, ".jpeg"))
    strcpy(filetype, "image/jpeg");
  else if (strstr(filename, ".css"))
    strcpy(filetype, "text/css");
  else if (strstr(filename, ".js"))
    strcpy(filetype, "application/javascript");
  else if (strstr(filename, ".mpg") || strstr(filename, ".mpeg"))
    strcpy(filetype, "video/mpeg");
  else
    strcpy(filetype, "text/plain");
}

// CGI 프로그램을 실행해서 동적 HTTP 200 응답을 만든다. CGI의 stdout이 클라이언트 응답 body가 된다.
void serve_dynamic(int fd, char *filename, char *cgiargs, int head_only)
{
  char buf[MAXLINE], *emptylist[] = {NULL};

  printf("[debug] cgi response: program=%s, query=%s\n", filename, cgiargs);

  snprintf(buf, sizeof(buf),
           "HTTP/1.0 200 OK\r\n"
           "Server: Tiny Web Server\r\n"
           "Connection: close\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (head_only)
  {
    char *head_end = "Content-type: text/html\r\n\r\n";
    Rio_writen(fd, head_end, strlen(head_end));
    return;
  }

  if (Fork() == 0)
  {
    printf("[debug] child process runs CGI now\n");
    setenv("QUERY_STRING", cgiargs, 1);
    Dup2(fd, STDOUT_FILENO);
    Close(fd);
    Execve(filename, emptylist, environ);
  }

  Wait(NULL);
}

// 에러 상태 코드와 HTML 에러 페이지를 응답으로 보낸다. HEAD 요청이면 body는 보내지 않는다.
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg, int head_only)
{
  char buf[MAXLINE], body[MAXBUF];

  printf("[debug] error response: %s %s, cause=%s\n", errnum, shortmsg, cause);

  snprintf(body, sizeof(body),
           "<html><title>Tiny Error</title>"
           "<body bgcolor=\"ffffff\">\r\n"
           "%s: %s\r\n"
           "<p>%s: %s\r\n"
           "<hr><em>The Tiny Web server</em>\r\n",
           errnum, shortmsg, longmsg, cause);

  snprintf(buf, sizeof(buf), "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  snprintf(buf, sizeof(buf), "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  snprintf(buf, sizeof(buf), "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));

  if (!head_only)
    Rio_writen(fd, body, strlen(body));
}
