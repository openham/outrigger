#include <stdlib.h>
#include "threads.h"

struct posix_thread_args {
	void	(*func)(void *);
	void	*arg;
};

void *posix_thread_wrapper(void *args)
{
	struct posix_thread_args	posix_args = *(struct posix_thread_args *)args;

	free(args);
	posix_args.func(posix_args.arg);
	return NULL;
}

int create_thread(void(*func)(void *), void *args, thread_t *thread)
{
	struct posix_thread_args	*posix_args = malloc(sizeof(struct posix_thread_args));

	posix_args->func = func;
	posix_args->arg = args;
	return pthread_create(thread, NULL, posix_thread_wrapper, posix_args);
}

int wait_thread(thread_t thread)
{
	void	*ret;

	return pthread_join(thread, &ret);
}
