#include "mutexes.h"

int mutex_init(mutex_t *mtx)
{
	return pthread_mutex_init(mtx, NULL);
}

int mutex_lock(mutex_t *mtx)
{
	return pthread_mutex_lock(mtx);
}

int mutex_unlock(mutex_t *mtx)
{
	return pthread_mutex_unlock(mtx);
}

int mutex_destroy(mutex_t *mtx)
{
	return pthread_mutex_destroy(mtx);
}
