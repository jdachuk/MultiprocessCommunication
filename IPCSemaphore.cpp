#include "IPCSemaphore.h"

#include <iostream>

IPCSemaphore::IPCSemaphore()
	: m_handle(nullptr)
{}

IPCSemaphore::~IPCSemaphore()
{
	CloseHandle(m_handle);
}

#define SPECIFIC (0x00000002L)

void IPCSemaphore::Init(const char* sName, size_t nInitial, size_t nMax, bool bReleaseRequired)
{
	sprintf_s(m_sName, "Local\\%s", sName);
	//memcpy(m_sName, sName, 256);

	DWORD dwDesiredRights = SEMAPHORE_ALL_ACCESS;
	if (bReleaseRequired) dwDesiredRights |= SEMAPHORE_MODIFY_STATE;

	//m_handle = OpenSemaphoreA(dwDesiredRights, TRUE, sName);
	//if (m_handle == nullptr && GetLastError() == ERROR_FILE_NOT_FOUND)
		//m_handle = CreateSemaphoreExA(NULL, (LONG)nInitial, (LONG)nMax, m_sName, 0, STANDARD_RIGHTS_ALL | SPECIFIC);
	m_handle = CreateSemaphoreExA(NULL, (LONG)nInitial, (LONG)nMax, m_sName, NULL, dwDesiredRights);
	std::cout << "Semaphore " << m_handle << " " << sName << " " << GetLastError() << std::endl;
}

void IPCSemaphore::Lock()
{
	std::cout << "Lock Semaphore " << m_sName << " handle " << m_handle << std::endl;
	if (WAIT_FAILED == WaitForSingleObject(m_handle, INFINITE)) std::cout << "LockSemaphore error " << GetLastError() << std::endl;
}

void IPCSemaphore::Release()
{
	std::cout << "Release Semaphore " << m_sName << " handle " << m_handle << std::endl;
	if (!ReleaseSemaphore(m_handle, 1, NULL)) std::cout << "ReleaseSemaphore error " << GetLastError() << std::endl;
}
