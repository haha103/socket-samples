#include "socket-utils.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char ** argv)
{
	int sfd, n;
	char send_buf[SOCK_BUF_SIZ];
	char recv_buf[SOCK_BUF_SIZ];

	if (argc < 3) {
		fprintf(stderr, "Usage: %s [server ip address] [server port]\n", argv[0]);
		return EXIT_FAILURE;
	}

	if ((sfd = create_and_connect(argv[1], argv[2])) == -1) {
		perror("socket initialization failed");
		return EXIT_FAILURE;
	}

	while (fgets(send_buf, SOCK_BUF_SIZ, stdin) != NULL) {
		if (write(sfd, send_buf, strlen(send_buf)) == -1)
			perror("write");
		if ((n = read(sfd, recv_buf, SOCK_BUF_SIZ)) == -1)
			perror("read");
		else {
			recv_buf[n] = '\0';
			puts(recv_buf);
		}
	}
	
	return EXIT_SUCCESS;
}
