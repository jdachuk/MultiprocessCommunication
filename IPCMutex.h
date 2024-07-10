#pragma once

#include <Windows.h>

#include <string>
#include <memory>

class IPCMutex
{
public:
	IPCMutex(const std::string& name);
	~IPCMutex();

	void lock();
	void unlock();

private:
	HANDLE m_handle;
};

