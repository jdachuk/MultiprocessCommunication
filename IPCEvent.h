#pragma once
#include <Windows.h>
#include <string>

class IPCEvent
{
public:
	IPCEvent(const std::string& name);
	~IPCEvent();

	DWORD Wait(DWORD millis = INFINITE);

	void Notify();

private:
	HANDLE m_EventHandle;
};

