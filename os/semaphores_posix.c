#include "semaphores.h"

int semaphore_init(semaphore_t *sem, unsigned init)
{
	return sem_init(sem, 0, init);
}

int semaphore_post(semaphore_t *sem)
{
	return sem_post(sem);
}

int semaphore_wait(semaphore_t *sem)
{
	return sem_wait(sem);
}

int semaphore_destroy(semaphore_t *sem)
{
	return sem_destroy(sem);
}
