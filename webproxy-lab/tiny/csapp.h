// csapp.h - CS:APP 책에서 제공하는 함수 원형과 상수 모음
#ifndef __CSAPP_H__
#define __CSAPP_H__

#include <stdio.h>      // printf, fprintf, FILE 같은 표준 입출력 함수
#include <stdlib.h>     // exit, malloc, free 같은 기본 유틸 함수
#include <stdarg.h>     // 인자 개수가 정해지지 않은 함수 처리
#include <unistd.h>     // read, write, close, fork 같은 POSIX 함수
#include <string.h>     // strlen, strcpy, memset 같은 문자열/메모리 함수
#include <ctype.h>      // isdigit, isspace 같은 문자 검사 함수
#include <setjmp.h>     // setjmp, longjmp 같은 비지역 점프 기능
#include <signal.h>     // signal, sigaction 같은 시그널 처리 기능
#include <dirent.h>     // opendir, readdir 같은 디렉터리 탐색 함수
#include <sys/time.h>   // timeval 구조체와 시간 관련 기능
#include <sys/types.h>  // pid_t, size_t 같은 시스템 자료형
#include <sys/wait.h>   // wait, waitpid 같은 자식 프로세스 대기 함수
#include <sys/stat.h>   // stat, fstat 같은 파일 상태 확인 함수
#include <fcntl.h>      // open에서 쓰는 O_RDONLY, O_CREAT 같은 플래그
#include <sys/mman.h>   // mmap, munmap 같은 메모리 매핑 함수
#include <errno.h>      // 시스템 호출 실패 이유를 담는 errno
#include <math.h>       // sqrt, pow 같은 수학 함수
#include <pthread.h>    // pthread_create 같은 POSIX 스레드 함수
#include <semaphore.h>  // sem_init, sem_wait 같은 세마포어 함수
#include <sys/socket.h> // socket, bind, listen, accept 같은 소켓 함수
#include <netdb.h>      // getaddrinfo, getnameinfo 같은 주소 변환 함수
#include <netinet/in.h> // sockaddr_in 같은 인터넷 주소 구조체
#include <arpa/inet.h>  // inet_ntop, inet_pton 같은 IP 주소 변환 함수

// 새 파일을 만들 때 기본으로 줄 권한
#define DEF_MODE   S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH

// 기본 권한에서 그룹/기타 사용자 쓰기 권한을 빼기 위한 마스크
#define DEF_UMASK  S_IWGRP|S_IWOTH

// struct sockaddr를 짧게 SA라고 부르기 위한 별칭
typedef struct sockaddr SA;

// RIO 내부 버퍼 크기
#define RIO_BUFSIZE 8192

// RIO가 버퍼 상태를 기억하기 위해 쓰는 구조체
typedef struct {
    int rio_fd;                // 이 RIO 버퍼가 읽을 파일 디스크립터
    int rio_cnt;               // 내부 버퍼에 아직 남아 있는 바이트 수
    char *rio_bufptr;          // 내부 버퍼에서 다음에 읽을 위치
    char rio_buf[RIO_BUFSIZE]; // 실제 내부 버퍼
} rio_t;

// DNS 오류 번호를 담는 전역 변수
extern int h_errno;

// 현재 프로세스의 환경 변수 목록
extern char **environ;

// 텍스트 한 줄의 최대 길이
#define	MAXLINE	 8192

// 일반 입출력 버퍼 크기
#define MAXBUF   8192

// listen에서 사용할 연결 대기 큐 크기
#define LISTENQ  1024

// Unix 시스템 호출 에러를 출력하고 종료
void unix_error(char *msg);

// POSIX 함수 에러 코드를 출력하고 종료
void posix_error(int code, char *msg);

// 예전 DNS 함수 에러를 출력하고 종료
void dns_error(char *msg);

// getaddrinfo 계열 에러를 출력하고 종료
void gai_error(int code, char *msg);

// 프로그램 자체 에러 메시지를 출력하고 종료
void app_error(char *msg);

// fork를 감싼 함수, 실패하면 에러 처리
pid_t Fork(void);

// 현재 프로세스를 다른 실행 파일로 바꿈
void Execve(const char *filename, char *const argv[], char *const envp[]);

// 자식 프로세스 하나가 끝날 때까지 기다림
pid_t Wait(int *status);

// 특정 자식 프로세스의 상태 변화를 기다림
pid_t Waitpid(pid_t pid, int *iptr, int options);

// 특정 프로세스에 시그널을 보냄
void Kill(pid_t pid, int signum);

// 지정한 초만큼 잠깐 멈춤
unsigned int Sleep(unsigned int secs);

// 시그널이 올 때까지 멈춤
void Pause(void);

// 지정한 시간 뒤 SIGALRM이 오도록 예약
unsigned int Alarm(unsigned int seconds);

// 프로세스 그룹 ID를 설정
void Setpgid(pid_t pid, pid_t pgid);

// 현재 프로세스 그룹 ID를 가져옴
pid_t Getpgrp();

// 시그널 핸들러 함수 모양
typedef void handler_t(int);

// 시그널 핸들러를 등록
handler_t *Signal(int signum, handler_t *handler);

// 현재 스레드의 시그널 마스크를 바꿈
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

// 빈 시그널 집합을 만듦
void Sigemptyset(sigset_t *set);

// 모든 시그널이 들어 있는 집합을 만듦
void Sigfillset(sigset_t *set);

// 시그널 집합에 특정 시그널을 추가
void Sigaddset(sigset_t *set, int signum);

// 시그널 집합에서 특정 시그널을 제거
void Sigdelset(sigset_t *set, int signum);

// 특정 시그널이 집합에 들어 있는지 확인
int Sigismember(const sigset_t *set, int signum);

// 임시 시그널 마스크로 바꾸고 시그널을 기다림
int Sigsuspend(const sigset_t *set);

// 시그널 핸들러 안에서도 안전하게 문자열 출력
ssize_t sio_puts(char s[]);

// 시그널 핸들러 안에서도 안전하게 long 값 출력
ssize_t sio_putl(long v);

// 시그널 안전 방식으로 에러 출력 후 종료
void sio_error(char s[]);

// sio_puts의 에러 처리 래퍼
ssize_t Sio_puts(char s[]);

// sio_putl의 에러 처리 래퍼
ssize_t Sio_putl(long v);

// sio_error의 래퍼
void Sio_error(char s[]);

// 파일을 열고 파일 디스크립터를 반환
int Open(const char *pathname, int flags, mode_t mode);

// 파일 디스크립터에서 바이트를 읽음
ssize_t Read(int fd, void *buf, size_t count);

// 파일 디스크립터에 바이트를 씀
ssize_t Write(int fd, const void *buf, size_t count);

// 파일에서 읽고 쓸 위치를 이동
off_t Lseek(int fildes, off_t offset, int whence);

// 파일 디스크립터를 닫음
void Close(int fd);

// 여러 파일 디스크립터 중 준비된 것이 있는지 기다림
int Select(int  n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	   struct timeval *timeout);

// 파일 디스크립터 번호를 복제
int Dup2(int fd1, int fd2);

// 파일 경로로 파일 상태 정보를 가져옴
void Stat(const char *filename, struct stat *buf);

// 파일 디스크립터로 파일 상태 정보를 가져옴
void Fstat(int fd, struct stat *buf);

// 디렉터리를 엶
DIR *Opendir(const char *name);

// 디렉터리 항목을 하나 읽음
struct dirent *Readdir(DIR *dirp);

// 디렉터리를 닫음
int Closedir(DIR *dirp);

// 파일이나 메모리를 가상 메모리 주소에 매핑
void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);

// mmap으로 만든 매핑을 해제
void Munmap(void *start, size_t length);

// FILE 스트림을 닫음
void Fclose(FILE *fp);

// 파일 디스크립터를 FILE 스트림으로 바꿈
FILE *Fdopen(int fd, const char *type);

// FILE 스트림에서 문자열 한 줄을 읽음
char *Fgets(char *ptr, int n, FILE *stream);

// 파일을 FILE 스트림으로 엶
FILE *Fopen(const char *filename, const char *mode);

// FILE 스트림에 문자열을 씀
void Fputs(const char *ptr, FILE *stream);

// FILE 스트림에서 블록 단위로 읽음
size_t Fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

// FILE 스트림에 블록 단위로 씀
void Fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

// 동적 메모리를 할당
void *Malloc(size_t size);

// 이미 할당한 메모리 크기를 바꿈
void *Realloc(void *ptr, size_t size);

// 0으로 초기화된 동적 메모리를 할당
void *Calloc(size_t nmemb, size_t size);

// 동적 메모리를 해제
void Free(void *ptr);

// 소켓을 하나 만듦
int Socket(int domain, int type, int protocol);

// 소켓 옵션을 설정
void Setsockopt(int s, int level, int optname, const void *optval, int optlen);

// 소켓에 주소와 포트를 묶음
void Bind(int sockfd, struct sockaddr *my_addr, int addrlen);

// 소켓을 클라이언트 연결 요청 대기 상태로 만듦
void Listen(int s, int backlog);

// 클라이언트 연결을 받아 새 연결 소켓을 반환
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);

// 서버 소켓에 연결
void Connect(int sockfd, struct sockaddr *serv_addr, int addrlen);

// 호스트 이름과 포트를 소켓 주소 정보로 바꿈
void Getaddrinfo(const char *node, const char *service,
                 const struct addrinfo *hints, struct addrinfo **res);

// 소켓 주소를 호스트 이름과 서비스 이름으로 바꿈
void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host,
                 size_t hostlen, char *serv, size_t servlen, int flags);

// getaddrinfo가 만든 주소 목록을 해제
void Freeaddrinfo(struct addrinfo *res);

// 네트워크 주소를 문자열 IP 주소로 바꿈
void Inet_ntop(int af, const void *src, char *dst, socklen_t size);

// 문자열 IP 주소를 네트워크 주소로 바꿈
void Inet_pton(int af, const char *src, void *dst);

// 호스트 이름으로 주소 정보를 찾음
struct hostent *Gethostbyname(const char *name);

// 주소로 호스트 정보를 찾음
struct hostent *Gethostbyaddr(const char *addr, int len, int type);

// 새 스레드를 만들어 지정한 함수를 실행
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
		    void * (*routine)(void *), void *argp);

// 스레드가 끝날 때까지 기다리고 반환값을 받음
void Pthread_join(pthread_t tid, void **thread_return);

// 스레드에 취소 요청을 보냄
void Pthread_cancel(pthread_t tid);

// 스레드 종료 자원을 자동으로 회수하게 분리
void Pthread_detach(pthread_t tid);

// 현재 스레드를 종료
void Pthread_exit(void *retval);

// 현재 스레드 ID를 반환
pthread_t Pthread_self(void);

// 초기화 함수를 딱 한 번만 실행
void Pthread_once(pthread_once_t *once_control, void (*init_function)());

// 세마포어를 초기화
void Sem_init(sem_t *sem, int pshared, unsigned int value);

// 세마포어 값을 감소시키고 필요하면 기다림
void P(sem_t *sem);

// 세마포어 값을 증가시키고 기다리는 스레드를 깨움
void V(sem_t *sem);

// fd에서 정확히 n바이트를 읽으려고 시도
ssize_t rio_readn(int fd, void *usrbuf, size_t n);

// fd에 정확히 n바이트를 쓰려고 시도
ssize_t rio_writen(int fd, void *usrbuf, size_t n);

// 버퍼를 사용하는 RIO 읽기 상태를 초기화
void rio_readinitb(rio_t *rp, int fd);

// RIO 버퍼를 통해 최대 n바이트를 읽음
ssize_t	rio_readnb(rio_t *rp, void *usrbuf, size_t n);

// RIO 버퍼를 통해 한 줄을 읽음
ssize_t	rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

// rio_readn의 에러 처리 래퍼
ssize_t Rio_readn(int fd, void *usrbuf, size_t n);

// rio_writen의 에러 처리 래퍼
void Rio_writen(int fd, void *usrbuf, size_t n);

// rio_readinitb의 래퍼
void Rio_readinitb(rio_t *rp, int fd);

// rio_readnb의 에러 처리 래퍼
ssize_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n);

// rio_readlineb의 에러 처리 래퍼
ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

// 서버에 연결하는 클라이언트 소켓을 만듦
int open_clientfd(char *hostname, char *port);

// 클라이언트 연결을 기다리는 listen 소켓을 만듦
int open_listenfd(char *port);

// open_clientfd의 에러 처리 래퍼
int Open_clientfd(char *hostname, char *port);

// open_listenfd의 에러 처리 래퍼
int Open_listenfd(char *port);

#endif
