#include "IPCConditionalVariable.h"

#include <exception>
#include <stdio.h>

#include <iostream>

IPCConditionalVariable::~IPCConditionalVariable()
{
	CloseHandle(m_event);
}

void IPCConditionalVariable::Init(const char* sBaseName, bool bWillSignal)
{
	char sName[256];
	sprintf_s(sName, sizeof(sName), "Local\\Event_%s", sBaseName);

	DWORD dwRights = EVENT_ALL_ACCESS;
	if (bWillSignal) dwRights |= EVENT_MODIFY_STATE;
	//else dwRights |= 0x3;

	m_event = OpenEventA(dwRights, TRUE, sName);
	if (NULL == m_event && ERROR_FILE_NOT_FOUND == GetLastError())
		m_event = CreateEventExA(NULL, sName, 0, dwRights);
	if (NULL == m_event) std::cout << "Failed to create event " << GetLastError() << std::endl;

	std::cout << "Event " << m_event << " " << sName << " " << GetLastError() << std::endl;
}

DWORD IPCConditionalVariable::Wait(DWORD dwMilliseconds)
{
	auto rt = WaitForSingleObjectEx(m_event, dwMilliseconds, FALSE);
	std::cout << m_event << " Got signal " << rt << std::endl;
	return rt;
}

DWORD IPCConditionalVariable::Wait(std::function<bool()> cond)
{
	DWORD rt = ERROR_SUCCESS;
	while (!cond())
	{
		rt = WaitForSingleObjectEx(m_event, INFINITE, FALSE);
	}
	return rt;
}

void IPCConditionalVariable::NotifyOne()
{
	std::cout << "Signal " << m_event << std::endl;
	if (!SetEvent(m_event)) std::cout << m_event << " SetEvent " << GetLastError() << std::endl;
}
