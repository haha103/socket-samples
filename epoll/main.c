#include "socket-utils.h"

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#define MAXEVENTS 64

int main (int argc, char ** argv)
{
	int sfd, s;
	int efd;

	struct epoll_event event;
	struct epoll_event * events;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((sfd = create_and_bind(argv[1])) == -1)
		abort();

	if ((s = make_socket_non_blocking(sfd)) == -1)
		abort();

	if ((s = listen(sfd, SOMAXCONN)) == -1) {
		perror("listen");
		abort();
	}

	if ((efd = epoll_create1(0)) == -1) {
		perror("epoll_create1");
		abort();
	}

	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;
	if ((s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event)) == -1) {
		perror("epoll_ctl");
		abort();
	}

	events = calloc(MAXEVENTS, sizeof (event));

	// the event loop

	while (1) {
		int n = epoll_wait(efd, events, MAXEVENTS, -1);
		for (int i = 0; i < n; ++i) {
			struct epoll_event e = events[i];
			if ((e.events & EPOLLERR) ||
					(e.events & EPOLLHUP) ||
					(!(e.events & EPOLLIN))) {
				fprintf(stderr, "epoll error\n");
				close(e.data.fd);
				continue;
			} else if (sfd == e.data.fd) {
				// we have a notification on the listening connection
				while (1) {
					struct sockaddr in_addr;
					socklen_t in_len = sizeof (in_addr);
					int infd;
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
					if ((infd = accept(sfd, &in_addr, &in_len)) == -1) {
						if ((errno == EAGAIN) ||
								(errno == EWOULDBLOCK)) {
							break; // we've processed all incoming connections
						} else {
							perror("accept");
							break;
						}
					}

					if ((s = getnameinfo(&in_addr, in_len, hbuf, sizeof (hbuf), sbuf, sizeof (sbuf), NI_NUMERICHOST | NI_NUMERICSERV)) == 0) {
						printf("Accepted connection on descriptor %d (host = %s, port = %s)\n", infd, hbuf, sbuf);
					}

					if ((s = make_socket_non_blocking(infd)) == -1)
						abort();

					event.data.fd = infd;
					event.events = EPOLLIN | EPOLLET;
					if ((s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event)) == -1) {
						perror("epoll_ctl");
						abort();
					}
				}
				continue;
			} else { // we have data on the fd waiting to be read.
				int done = 0;
				while (1) {
					ssize_t count;
					char buf[SOCK_BUF_SIZ];
					count = read(e.data.fd, buf, sizeof (buf));
					if (count == -1) {
						if (errno != EAGAIN) {
							perror("read");
							done = 1;
						}
						break;
					} else if (count == 0) {
						done = 1;
						break;
					}

					// echo back
					if ((s = write(e.data.fd, buf, count)) == -1) {
						perror("write");
						abort();
					}
					// log to own stdout
					if ((s = write(1, buf, count)) == -1) {
						perror("write");
						abort();
					}
				}

				if (done) {
					printf("Closed connection on descriptor %d\n", e.data.fd);
					// close the descriptor will make epoll remove it from
					// the set of descriptors which are monitored
					close(e.data.fd);
				}
			}
		}
	}

	free(events);
	close(sfd);
	
	return EXIT_SUCCESS;
}










