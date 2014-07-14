#include "semaphores.h"

int semaphore_init(semaphore_t *sem, unsigned init)
{
	*sem = CreateSemaphore(NULL, init, LONG_MAX, NULL);
	if (*sem == NULL)
		return -1;
	return 0;
}

int semaphore_post(semaphore_t *sem)
{
	if (!ReleaseSemaphore(*sem, 1, NULL))
		return -1;
	return 0;
}

int semaphore_wait(semaphore_t *sem)
{
	if (WaitForSingleObject(*sem, INFINITE) != WAIT_OBJECT_0)
		return -1;
	return 0;
}

int semaphore_destroy(semaphore_t *sem)
{
	if (CloseHandle(*sem) == 0)
		return -1;
	return 0;
}
