#include <stdlib.h>
#include "threads.h"

struct win32_thread_args {
	void	(*func)(void *);
	void	*arg;
};

unsigned __stdcall win32_thread_wrapper(void *args)
{
	struct win32_thread_args	win32_args = *(struct win32_thread_args *)args;

	free(args);
	win32_args.func(win32_args.arg);
	return 0;
}

int create_thread(void(*func)(void *), void *args, thread_t *thread)
{
	struct win32_thread_args	*win32_args = malloc(sizeof(struct win32_thread_args));

	win32_args->func = func;
	win32_args->arg = args;
	*thread = (HANDLE)_beginthreadex(NULL, 0, win32_thread_wrapper, win32_args, 0, NULL);
	if (*thread == 0)
		return -1;
	return 0;
}

int wait_thread(thread_t thread)
{
	if (WaitForSingleObject(thread, INFINITE) != 0)
		return -1;
	if (CloseHandle(thread) == 0)
		return -1;
	return 0;
}
