#ifndef _SOCKET_UTILS_H_
#define _SOCKET_UTILS_H_

#define SOCK_BUF_SIZ 256

int create_and_bind (const char * port);
int make_socket_non_blocking(int sfd);

int create_and_connect (const char * addr, const char * port);

#endif
