#ifndef SOCKETS_H
#define SOCKETS_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define ioctl ioctlsocket
#else
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#define closesocket close
#endif

#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

int socket_nonblocking(int s);

#endif