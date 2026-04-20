CC = gcc
CFLAGS = -O0 -Wall -Wextra -g

COMMON_OBJS = echo_io.o

all: echo_client echo_server

echo_client: echo_client.o open_clientfd.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o echo_client echo_client.o open_clientfd.o $(COMMON_OBJS)

echo_server: echo_server.o open_listenfd.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o echo_server echo_server.o open_listenfd.o $(COMMON_OBJS)

echo_client.o: echo_client.c echo.h
	$(CC) $(CFLAGS) -c echo_client.c

echo_server.o: echo_server.c echo.h
	$(CC) $(CFLAGS) -c echo_server.c

open_clientfd.o: open_clientfd.c echo.h
	$(CC) $(CFLAGS) -c open_clientfd.c

open_listenfd.o: open_listenfd.c echo.h
	$(CC) $(CFLAGS) -c open_listenfd.c

echo_io.o: echo_io.c echo.h
	$(CC) $(CFLAGS) -c echo_io.c

clean:
	rm -f *.o echo_client echo_server
