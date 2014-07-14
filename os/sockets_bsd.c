#include <fcntl.h>

#include "sockets.h"

int socket_nonblocking(int s)
{
	return fcntl(s, F_SETFL, O_NONBLOCK)==-1?-1:0;
}
