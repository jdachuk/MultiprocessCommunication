#pragma once
#include <windows.h>

#include <array>
#include <string>

#include "Constants.h"

class Actor;
class Buffer;

class Administrator
{
	struct FileAdminSector
	{
		std::array<bool, MAX_MAPPING_OBJECT_USERS> userAvailablePool;
		std::array<bool, MAX_MAPPING_BUFFERS> bufferAvailablePool;
	};
public:
	bool Init(LPCTSTR szFileName);

	Actor* CreateActor(const std::string& strSrcPath, const std::string& strDstPath);

private:
	static size_t GetNumOfActiveUsers(FileAdminSector* pAdminSector);
	static size_t GetAvailableUserIdx(FileAdminSector* pAdminSector);
	static size_t GetAddressForActor(size_t nUserIdx);

	static size_t GetAvailableBufferIdx(FileAdminSector* pAdminSector);
	static size_t GetAddressForBuffer(size_t nBufferIdx);

private:
	LPCTSTR m_szFileName;
	LPCTSTR m_szMutexName;
	HANDLE m_mutexHandle;
	HANDLE m_fileHandle;
	HANDLE m_buffHandle;

	bool m_bInitialized;
};

