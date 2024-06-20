#include "IPCScopedLock.h"

IPCScopedLock::IPCScopedLock(IPCMutex* mutex)
	: m_mutex(mutex)
{
	m_mutex->Lock();
}

IPCScopedLock::~IPCScopedLock()
{
	m_mutex->Unlock();
}
