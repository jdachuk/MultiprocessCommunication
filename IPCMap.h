#pragma once
#include <Windows.h>

#include <string>

class IPCMap
{
public:
	IPCMap(size_t fileSize, const std::string& fileName);
	~IPCMap();

	bool IsCreated() const;

	void* GetHandle();

private:
	bool m_FileExisted;
	HANDLE m_FileMappingHandle;
	HANDLE m_MapViewHandle;
};

