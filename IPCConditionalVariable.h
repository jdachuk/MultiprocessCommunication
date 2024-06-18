#pragma once
#include <windows.h>

#include <stdio.h>

#include <exception>
#include <functional>

#include <iostream>

class IPCConditionalVariable
{
public:
	~IPCConditionalVariable()
	{
		if (nullptr != m_mutex)
			CloseHandle(m_mutex);
		if (nullptr != m_event)
			CloseHandle(m_event);
	}

	void Init(const char* sBaseName)
	{
		char sName[256];

		sprintf_s(sName, sizeof(sName) / sizeof(char), "Mutex_%s", sBaseName);
		m_mutex = OpenMutexA(SYNCHRONIZE, TRUE, sName);
		if (nullptr == m_mutex && ERROR_FILE_NOT_FOUND == GetLastError())
			m_mutex = CreateMutexA(NULL, TRUE, sName);
		if (nullptr == m_mutex)
			throw std::exception("Cannot create mutex");

		sprintf_s(sName, sizeof(sName) / sizeof(char), "Event_%s", sBaseName);
		m_event = CreateEventA(NULL, FALSE, FALSE, sName);
		if (nullptr == m_event)
			throw std::exception("Cannot create event");
	}

	void Wait(std::function<bool()> cond = []() { return true; })
	{
		while (!cond())
		{
			SignalObjectAndWait(m_mutex, m_event, INFINITE, FALSE);
		}
	}

	void NotifyOne()
	{
		WaitForSingleObject(m_mutex, INFINITE);
		SetEvent(m_event);
		ResetEvent(m_event);
		ReleaseMutex(m_mutex);
	}

private:
	HANDLE m_mutex;
	HANDLE m_event;
};
