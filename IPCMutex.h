#pragma once
#include <windows.h>

#include <exception>

#include <iostream>

class IPCMutex
{
public:
	IPCMutex()
		: m_handle(nullptr)
	{}

	void Init(const char* sName)
	{
		m_handle = CreateMutexExA(NULL, sName, 0, SYNCHRONIZE);
		if (nullptr == m_handle)
			throw std::exception("Cannot create mutex");
	}

	~IPCMutex()
	{
		if (nullptr != m_handle)
		{
			ReleaseMutex(m_handle);
			CloseHandle(m_handle);
		}
	}

	void Lock()
	{
		WaitForSingleObject(m_handle, INFINITE);
	}

	void Unlock()
	{
		ReleaseMutex(m_handle);
	}

	HANDLE GetHandle()
	{
		return m_handle;
	}

private:
	HANDLE m_handle;
};