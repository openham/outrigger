#ifndef THREADS_H
#define THREADS_H

#ifdef WIN32_THREADS
#include <Windows.h>
#include <process.h>
typedef	HANDLE	thread_t;
#endif

#ifdef POSIX_THREADS
#include <pthread.h>
typedef pthread_t	thread_t;
#endif

int create_thread(void(*)(void *), void *args, thread_t *);
int wait_thread(thread_t);

#endif