#pragma once
#include <windows.h>

#include "Globals.h"
#include "IPCMutex.h"

class Actor;

struct FileAdminSector
{
	std::array<bool, MAX_MAPPING_OBJECT_USERS> userAvailablePool;
	std::array<bool, MAX_MAPPING_BUFFERS> bufferAvailablePool;
};

class Administrator
{
public:
	bool Init(const char* szFileName);

	Actor* CreateActor(const char strSrcPath[MAX_PATH_LENGTH], const char strDstPath[MAX_PATH_LENGTH]);
	Actor* GetActorPtr(size_t nActorId) const;
	BufferType* GetBufferPtr(size_t nBufferId) const;

private:
	static size_t GetNumOfActiveUsers(FileAdminSector* pAdminSector);
	static size_t GetAvailableUserIdx(FileAdminSector* pAdminSector);
	static size_t GetAddressForActor(size_t nUserIdx);

	static size_t GetAvailableBufferIdx(FileAdminSector* pAdminSector);
	static size_t GetAddressForBuffer(size_t nBufferIdx);

private:
	bool m_bInitialized = false;
	bool m_bRunning = false;

	const char* m_szFileName = nullptr;
	HANDLE m_fileHandle = nullptr;
	HANDLE m_buffHandle = nullptr;

	mutable IPCMutex m_mutex;
};
