#include "IPCMutex.h"

#include <exception>

IPCMutex::IPCMutex(const std::string& name)
{
	char mutexName[MAX_PATH];
	sprintf_s(mutexName, "Mutex_%s", name.data());

	m_handle = CreateMutexExA(nullptr, mutexName, 0, MUTEX_ALL_ACCESS | MUTEX_MODIFY_STATE);
	if (nullptr == m_handle)
	{
		throw std::exception("Couldn't create mutex.");
	}
}

IPCMutex::~IPCMutex()
{
	ReleaseMutex(m_handle);
	CloseHandle(m_handle);
}

void IPCMutex::lock()
{
	WaitForSingleObject(m_handle, INFINITE);
}

void IPCMutex::unlock()
{
	ReleaseMutex(m_handle);
}
