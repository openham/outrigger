#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#ifdef WIN32_SEMAPHORES
#include <Windows.h>
typedef HANDLE semaphore_t;
#endif

#ifdef POSIX_SEMAPHORES
#include <semaphore.h>
typedef sem_t semaphore_t
#endif

int semaphore_init(semaphore_t *, unsigned);
int semaphore_post(semaphore_t *);
int semaphore_wait(semaphore_t *);
int semaphore_destroy(semaphore_t *);

#endif