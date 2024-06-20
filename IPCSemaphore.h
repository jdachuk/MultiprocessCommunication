#pragma once
#include <windows.h>

class IPCSemaphore
{
public:
	IPCSemaphore();
	~IPCSemaphore();

	void Init(const char* sName, size_t nInitial, size_t nMax, bool bReleaseRequired);

	void Lock();

	void Release();

private:
	HANDLE m_handle;
	char m_sName[256];
};

