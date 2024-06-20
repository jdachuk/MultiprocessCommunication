#pragma once
#include <windows.h>

class IPCMutex
{
public:
	IPCMutex();
	~IPCMutex();

	void Init(const char* sName);

	void Lock();
	void Unlock();

	HANDLE GetHandle();

private:
	HANDLE m_handle;
};