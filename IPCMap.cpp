#include "IPCMap.h"

IPCMap::IPCMap(size_t fileSize, const std::string& fileName)
	: m_FileExisted(true)
	, m_FileMappingHandle(nullptr)
	, m_MapViewHandle(nullptr)
{
	m_FileMappingHandle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, fileName.c_str());

	if (nullptr == m_FileMappingHandle && ERROR_FILE_NOT_FOUND == GetLastError())
	{
		m_FileExisted = false;
		m_FileMappingHandle = CreateFileMappingA(INVALID_HANDLE_VALUE,
			nullptr, PAGE_READWRITE, 0, (DWORD)fileSize, fileName.c_str());
	}

	if (nullptr == m_FileMappingHandle)
	{
		throw std::exception("Failed to open file mapping!");
	}

	m_MapViewHandle = MapViewOfFile(m_FileMappingHandle,
		FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, (DWORD)fileSize);

	if (nullptr == m_MapViewHandle)
	{
		throw std::exception("Failed to map view of file!");
	}
}

IPCMap::~IPCMap()
{
	if (nullptr != m_MapViewHandle)
	{
		UnmapViewOfFile(m_MapViewHandle);
	}
	if (nullptr != m_FileMappingHandle)
	{
		CloseHandle(m_FileMappingHandle);
	}
}

bool IPCMap::IsCreated() const
{
	return !m_FileExisted;
}

void* IPCMap::GetHandle()
{
	return m_MapViewHandle;
}
