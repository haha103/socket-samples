#include "socket-utils.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int make_socket_non_blocking (int sfd)
{
	int flags, s;

	if ((flags = fcntl(sfd, F_GETFL, 0)) == -1) {
		perror("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	if ((s = fcntl(sfd, F_SETFL, flags)) == -1) {
		perror("fcntl");
		return -1;
	}

	return 0;
}

int create_and_bind (const char * port)
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  int s, sfd;
	memset(&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((s = getaddrinfo(NULL, port, &hints, &result)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		if ((sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1)
			continue;
		if ((s = bind(sfd, rp->ai_addr, rp->ai_addrlen)) == 0) {
			// we managed to bind successfully!
			struct sockaddr_in * ipv4 = (struct sockaddr_in *)rp->ai_addr;
			char ipv4_addr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(ipv4->sin_addr), ipv4_addr, INET_ADDRSTRLEN);
			printf("Bind sucessfully to %s at port %hu\n", ipv4_addr, ntohs(ipv4->sin_port));
			break;
		}
		close(sfd);
	}

	if (rp == NULL) {
		fprintf(stderr, "Could not bind\n");
		return -1;
	}
	freeaddrinfo(result);
	return sfd;
}

int create_and_connect (const char * addr, const char * port)
{
	int sfd;

	struct sockaddr_in server_addr;

	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "socket creation failed ...");
		return -1;
	}

	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_port = htons(atoi(port));
	
	if (connect(sfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
		fprintf(stderr, "connect failed ...");
		return -1;
	}
	
	return sfd;
}










