CC = clang
CFLAGS = -Wall
LDFLAGS = -lpthread

SERVER_EXE = epoll-server
CLIENT_EXE = epoll-client

all: $(SERVER_EXE) $(CLIENT_EXE)

$(CLIENT_EXE): main-client.o socket-utils.o
	$(CC) $(LDFLAGS) -o $@ $^

$(SERVER_EXE): main.o socket-utils.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(SERVER_EXE) $(CLIENT_EXE)



