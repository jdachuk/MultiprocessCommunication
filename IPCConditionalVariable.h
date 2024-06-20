#pragma once
#include <windows.h>

#include <functional>

class IPCConditionalVariable
{
	IPCConditionalVariable(IPCConditionalVariable&&) = delete;
public:
	IPCConditionalVariable() = default;
	~IPCConditionalVariable();

	void Init(const char* sBaseName, bool bWillSignal);

	DWORD Wait(DWORD dwMilliseconds=INFINITE);
	DWORD Wait(std::function<bool()> cond);

	void NotifyOne();

private:
	HANDLE m_event;
};
