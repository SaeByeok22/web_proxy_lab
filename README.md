# Week8 Task

### 11.6.(c) TINY의 출력을 조사해서 여러분이 사용하는 브라우저의 HTTP 버전을 결정하라.

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

### 11.7 TINY를 확장해서 MPG 비디오 파일을 처리하도록 하시오. 실제 브라우저를 사용해서 여러분의 결과를 체크하시오.

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

### 11.9 TINY를 수정해서 정적 컨텐츠를 처리할 때 요청한 파일을 MMAP과 rio_readn 대신에 malloc, rio_readn, rio_writen을 사용해서 연결 식별자에게 복사하도록 하시오.

### 11.10
#### A. 그림 11.27의 CGI adder 함수에 대한 HTML 형식을 작성하시오, 이 형식은 사용자가 함께 더할 두 개의 숫자로 채우는 두 개의 텍스트 상자를 포함해야 한다. 여러분의 형식은 GET 메소드를 사용해서 컨텐츠를 요청해야 한다.

#### B. 실제 브라우저를 사용해서 TINY로 부터 이 형식을 요청하고, 채운 형식을 TINY에 보내고, adder가 생성한 동적 컨텐츠를 표시하는 방법으로 여러분의 작업을 체크하라.

### 11.11 TINY를 확장해서 HTTP HEAD 메소드를 지원하도록 하라. TELNET을 웹 클라이언트로 사용해서 작업 결과를 체크하시오.