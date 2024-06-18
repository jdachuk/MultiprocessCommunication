#pragma once
#include <windows.h>

#include <array>
#include <string>

#include "Globals.h"
#include "IPCMutex.h"

template<typename T, size_t N>
class Actor;
template<typename T, size_t N>
class Buffer;

struct FileAdminSector
{
	std::array<bool, MAX_MAPPING_OBJECT_USERS> userAvailablePool;
	std::array<bool, MAX_MAPPING_BUFFERS> bufferAvailablePool;
};

template<typename T, size_t N>
class Administrator
{
public:
	bool Init(const char* szFileName);

	void Run();

	Actor<T, N>* CreateActor(const char strSrcPath[MAX_PATH_LENGTH], const char strDstPath[MAX_PATH_LENGTH]);
	Actor<T, N>* GetActorPtr(size_t nActorId) const;
	Buffer<T, N>* GetBufferPtr(size_t nBufferId) const;

private:
	static size_t GetNumOfActiveUsers(FileAdminSector* pAdminSector);
	static size_t GetAvailableUserIdx(FileAdminSector* pAdminSector);
	static size_t GetAddressForActor(size_t nUserIdx);

	static size_t GetAvailableBufferIdx(FileAdminSector* pAdminSector);
	static size_t GetAddressForBuffer(size_t nBufferIdx);

private:
	bool m_bInitialized;
	bool m_bRunning;

	const char* m_szFileName;
	HANDLE m_fileHandle;
	HANDLE m_buffHandle;

	mutable IPCMutex m_mutex;
};

#include "Administrator.inl"
