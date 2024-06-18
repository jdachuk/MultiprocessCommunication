#pragma once
#include "IPCMutex.h"

class IPCScopedLock
{
public:
	IPCScopedLock(IPCMutex& mutex)
		: m_mutex(mutex)
	{
		m_mutex.Lock();
	}

	~IPCScopedLock()
	{
		m_mutex.Unlock();
	}

private:
	IPCMutex& m_mutex;
};