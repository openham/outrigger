#include <sockets.h>

int socket_nonblocking(int s)
{
	u_long	nonblocking = 1;

	return ioctlsocket(s, FIONBIO, &nonblocking) == SOCKET_ERROR?-1:0;
}
