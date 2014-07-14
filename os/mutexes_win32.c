#include "mutexes.h"

int mutex_init(mutex_t *mtx)
{
	*mtx = CreateMutex(NULL, FALSE, NULL);
	if (*mtx == NULL)
		return -1;
	return 0;
}

int mutex_lock(mutex_t *mtx)
{
	if (WaitForSingleObject(*mtx, INFINITE) != WAIT_OBJECT_0)
		return -1;
	return 0;
}

int mutex_unlock(mutex_t *mtx)
{
	if (ReleaseMutex(*mtx) == 0)
		return -1;
	return 0;
}

int mutex_destroy(mutex_t *mtx)
{
	if (CloseHandle(*mtx) == 0)
		return -1;
	return 0;
}
