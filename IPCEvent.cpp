#include "IPCEvent.h"

#include <exception>

IPCEvent::IPCEvent(const std::string& name)
	: m_EventHandle(nullptr)
{
	m_EventHandle = CreateEventExA(nullptr, name.c_str(), 0, EVENT_ALL_ACCESS | EVENT_MODIFY_STATE);

	if (nullptr == m_EventHandle)
		throw std::exception("Failed to create event");
}

IPCEvent::~IPCEvent()
{
	if (nullptr != m_EventHandle)
	{
		CloseHandle(m_EventHandle);
	}
}

DWORD IPCEvent::Wait(DWORD millis)
{
	return WaitForSingleObjectEx(m_EventHandle, millis, FALSE);
}

void IPCEvent::Notify()
{
	SetEvent(m_EventHandle);
}
