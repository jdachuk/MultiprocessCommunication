#pragma once
#include "IPCMutex.h"

class IPCScopedLock
{
public:
	IPCScopedLock(IPCMutex* mutex);
	~IPCScopedLock();

private:
	IPCMutex* m_mutex;
};