# Week8 Task

### 1. _11.6.(c)_ TINY의 출력을 조사해서 여러분이 사용하는 브라우저의 HTTP 버전을 결정하라.

- 관련 함수 :
```
// tiny.c
void doit(int fd) {
   ...

   if (sscanf(buf, "%s %s %s", method, uri, version) != 3) {
      clienterror(fd, buf, "400", "Bad Request",
                 "Tiny could not parse the request line", 0);
      return;
   }

   printf("[debug] method=%s, uri=%s, version=%s\n", method, uri, version);
}
```
만약, 브라우저가 요청을 보내면 터미널에 다음과 같이 찍히도록 함.

```
[debug] method=GET, uri=/, version=HTTP/1.1
```

여기서의 version이 브라우저가 사용하는 HTTP 버전임.

---

### 2. _11.7_ TINY를 확장해서 MPG 비디오 파일을 처리하도록 하시오. 실제 브라우저를 사용해서 여러분의 결과를 체크하시오.

- 관련 함수 :
```
// tiny.c
void get_filetype(char *filename, char *filetype) {
   ...

   else if (strstr(filename, ".mpg") || strstr(filename, ".mpeg"))
   strcpy(filetype, "video/mpeg");
}
```

cat.mpg 이나, jerry.mpeg 같은 파일을 요청하면, 응답 헤더에 아래 MIME type을 넣음.

```
Content-type: video/mpeg
```

- 테스트

```
curl -I http://localhost:8000/sample.mpg
```

---

### 3. _11.9_ TINY를 수정해서 정적 컨텐츠를 처리할 때 요청한 파일을 MMAP과 rio_readn 대신에 malloc, rio_readn, rio_writen을 사용해서 연결 식별자에게 복사하도록 하시오.

- 기존 방식 : 정적 파일을 보낼 때에 mmap()을 사용함. 이 mmap()은 파일을 메모리에 매핑해서 바로 보내는 방식임. 즉, 파일 내용을 read()로 따로 복사하지 않고, 파일을 메모리처럼 다룸.

```
open file 
-> mmap file into memory 
-> write memory to client 
-> munmap
```

- 바꾼 방식 : 메모리 버퍼를 만들고, 파일 내용을 read()로 그 버퍼에 복사한 뒤, 그 버퍼를 write()로 보냄.

- 구현 방법 : serve_static() 함수에서 Mmap() / Munmap() 을 제거하고, malloc, rio_readn, rio_writen으로 바꿈.

```
// serve_static()
Allocate memory for the file size using malloc
-> Read file contents into memory using rio_readn
-> Send to client using rio_writen
-> Release memory for free
```

- 장단점 비교

| | mmap() | malloc() |
| --- | --- | --- |
| 장점 | 1. 코드가 짧고 깔끔함 <br> 2. 파일 내용을 별도 버퍼로 복사하지 않아도 됨 <br> 3. 큰 파일 처리에서 OS의 가상 메모리 관리 장점을 활용 가능 <br> 4. read()를 여러 번 호출하지 않아도 됨 | 1. 흐름이 직관적임 <br> 2. 파일 I/O와 메모리 사용을 직접 확인하기 좋음. | 
| 단점 | 매핑 실패, 페이지 단위 동작 같은 OS 의존적인 느낌이 강함 | 1. 파일 내용을 메모리로 한 번 복사해야 함 <br> 2. 큰 파일일수록 메모리 사용량 부담이 커질 수 있음 <br> 3. mmap()보다 성능상 비효율적일 수 있음 |

---

### 4. _11.10_
#### A. 그림 11.27의 CGI adder 함수에 대한 HTML 형식을 작성하시오, 이 형식은 사용자가 함께 더할 두 개의 숫자로 채우는 두 개의 텍스트 상자를 포함해야 한다. 여러분의 형식은 GET 메소드를 사용해서 컨텐츠를 요청해야 한다.

#### B. 실제 브라우저를 사용해서 TINY로 부터 이 형식을 요청하고, 채운 형식을 TINY에 보내고, adder가 생성한 동적 컨텐츠를 표시하는 방법으로 여러분의 작업을 체크하라.

- 구현 내용 :
```
adder.html 파일 추가
-> <form action="/cgi-bin/adder" method="GET"> 형식으로 작성.
-> 입력값은 a, b라는 name으로 전달되도록 구성.
-> adder.c는 QUERY_STRING을 읽어서 두 숫자를 파싱하고 합계 계산.
-> ?123&456 형식뿐 아니라 ?a=123&b=456 형식도 처리할 수 있도록 수정.
```

- 관련(구현) 파일 :
```
- webproxy-lab/tiny/adder.html
- webproxy-lab/tiny/cgi-bin/adder.c
- webproxy-lab/tiny/tiny.c
```

- 결과:
```
The answer is: 123 + 456 = 579
```

--- 

### 5. _11.11_ TINY를 확장해서 HTTP HEAD 메소드를 지원하도록 하라. TELNET을 웹 클라이언트로 사용해서 작업 결과를 체크하시오.

- 구현 내용 :
```
// tiny.c
// doit()에서 HEAD method를 허용하도록 수정.
head_only = !strcasecmp(method, "HEAD");
if (strcasecmp(method, "GET") && strcasecmp(method, "HEAD"))
{
   clienterror(fd, method, "501", "Not Implemented", "Tiny only implements GET and HEAD", 0);
   return;
}

// serve_static()에서 HEAD 요청이면 파일 body를 보내지 않고 header만 전송하도록 함.
void serve_static(int fd, char *filename, int filesize, int head_only) {
   ...

   rio_writen(fd, buf, strlen(buf));
   printf("Response headers:\n%s", buf);

   if (head_only)
      return;
}

// serve_dynamic()에서도 HEAD 요청이면 CGI를 실행하지 않고 header만 응답하도록 처리함.
void serve_dynamic(int fd, char *filename, char *cgiargs, int head_only) {
   ...

   if (head_only) {
      char *head_end = "Content-type: text/html\r\n\r\n";
      Rio_writen(fd, head_end, strlen(head_end));
      return;
   }
}

// clienterror()에서도 HEAD 요청일 때는 에러 body를 보내지 않도록 함.
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg, int head_only) {
   if (!head_only)
      Rio_writen(fd, body, strlen(body));
}
```

- 동작 흐름 :
```
1. 클라이언트가 `HEAD / HTTP/1.0` 요청을 보낸다.
2. Tiny는 요청 라인을 파싱해서 method가 `HEAD`인지 확인한다.
3. 요청 대상이 정적 파일이면 `Content-length`, `Content-type` 등을 포함한 응답 헤더를 만든다.
4. 요청 대상이 CGI면 동적 응답용 헤더만 보낸다.
5. 본문 데이터는 전송하지 않고 연결을 종료한다.
```

- 결과 : 
```
HTTP/1.0 200 OK
Server: Tiny Web Server
Connection: close
Content-length: ...
Content-type: text/html
```