#include <sockets.h>

#ifdef _WIN32
int socket_nonblocking(int s)
{
	u_long	nonblocking = 1;

	return ioctlsocket(s, FIONBIO, &nonblocking) == SOCKET_ERROR?-1:0;
}
#else
int socket_nonblocking(int s)
{
	return fcntl(s, F_SETFL, O_NONBLOCK);
}
#endif