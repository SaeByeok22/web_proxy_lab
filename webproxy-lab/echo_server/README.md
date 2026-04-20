# echo server

사진의 작업 단위에 맞춰 파일을 나눴다.

- `open_clientfd.c`: `[WEEK8] 구현 - echo서버 - open_clientfd()`
- `open_listenfd.c`: `[WEEK8] 구현 - echo서버 - open_listenfd()`
- `echo_client.c`: `[WEEK8] 구현 - echo서버 - 클라이언트 main()`
- `echo_server.c`: `[WEEK8] 구현 - echo서버 - 서버 main()`, `서버 echo()`
- `echo_io.c`: echo client/server가 같이 쓰는 줄 단위 입출력 함수

## build

```bash
cd /workspace/webproxy_lab_docker/webproxy-lab/echo_server
make
```

## telnet test

터미널 1:

```bash
./echo_server 15213
```

터미널 2:

```bash
telnet localhost 15213
```

입력:

```text
hello
```

응답:

```text
hello
```

## client test

터미널 1:

```bash
./echo_server 15213
```

터미널 2:

```bash
./echo_client localhost 15213
```

입력한 줄이 그대로 다시 출력되면 성공이다.
