#include "IPCMutex.h"

#include <iostream>

IPCMutex::IPCMutex()
	: m_handle(nullptr)
{}

IPCMutex::~IPCMutex()
{
	//ReleaseMutex(m_handle);
	CloseHandle(m_handle);
}

void IPCMutex::Init(const char* sName)
{
	m_handle = OpenMutexA(MUTEX_ALL_ACCESS | MUTEX_MODIFY_STATE, TRUE, sName);
	if (NULL == m_handle && ERROR_FILE_NOT_FOUND == GetLastError())
		m_handle = CreateMutexExA(NULL, sName, 0, MUTEX_ALL_ACCESS | MUTEX_MODIFY_STATE);

	std::cout << "Mutex " << m_handle << " " << sName << " " << GetLastError() << std::endl;
}

void IPCMutex::Lock()
{
	WaitForSingleObjectEx(m_handle, INFINITE, FALSE);
}

void IPCMutex::Unlock()
{
	ReleaseMutex(m_handle);
}

HANDLE IPCMutex::GetHandle()
{
	return m_handle;
}
