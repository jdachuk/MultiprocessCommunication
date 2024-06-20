#include "Administrator.h"

#include "Actor.h"
#include "Buffer.h"

#include "IPCMutex.h"
#include "IPCScopedLock.h"

#include <iostream>

#define FILE_SIZE sizeof(FileAdminSector) + MAX_MAPPING_OBJECT_USERS * sizeof(Actor) + MAX_MAPPING_BUFFERS * sizeof(Buffer<ChunkInfoType, MAX_CHUNKS>)

bool Administrator::Init(const char* szFileName)
{
    char sMutexName[256];
    bool bFileExisted = true;

    sprintf_s(sMutexName, sizeof(sMutexName) / sizeof(char), "Mutex_%s", szFileName);
    m_mutex.Init(sMutexName);

    std::cout << m_mutex.GetHandle() << std::endl;

    m_szFileName = szFileName;

    m_fileHandle = OpenFileMappingA(
        FILE_MAP_ALL_ACCESS,   // read/write access
        FALSE,                 // do not inherit the name
        m_szFileName);         // name of mapping object

    if (m_fileHandle == NULL && ERROR_FILE_NOT_FOUND == GetLastError())
    {
        bFileExisted = false;

        m_fileHandle = CreateFileMappingA(
            INVALID_HANDLE_VALUE,    // use paging file
            NULL,                    // default security
            PAGE_READWRITE,          // read/write access
            0,                       // maximum object size (high-order DWORD)
            FILE_SIZE,               // maximum object size (low-order DWORD)
            m_szFileName);           // name of mapping object
    }

    std::cout << "FileMapping " << m_fileHandle << " " << m_szFileName << " " << GetLastError() << std::endl;

    if (m_fileHandle == NULL)
    {
        return false;
    }

    m_buffHandle = MapViewOfFile(m_fileHandle,   // handle to map object
        FILE_MAP_WRITE | FILE_MAP_READ,                     // read/write permission
        0,
        0,
        FILE_SIZE);

    if (m_buffHandle == NULL)
    {
        return false;
    }

    if (!bFileExisted)
    {
        IPCScopedLock lk(&m_mutex);

        FileAdminSector* pAdminSector = new ((byte*)m_buffHandle) FileAdminSector();

        for (size_t i = 0; i < pAdminSector->userAvailablePool.size(); ++i)
            pAdminSector->userAvailablePool[i] = true;
        for (size_t i = 0; i < pAdminSector->bufferAvailablePool.size(); ++i)
            pAdminSector->bufferAvailablePool[i] = true;
    }

    m_bInitialized = true;

    return true;
}

Actor* Administrator::CreateActor(const char strSrcPath[MAX_PATH_LENGTH], const char strDstPath[MAX_PATH_LENGTH])
{
    if (!m_bInitialized)
    {
        return nullptr;
    }

    std::cout << "CreateActor\n";

    IPCScopedLock lk(&m_mutex);

    FileAdminSector* pAdminSector = (FileAdminSector*)((byte*)m_buffHandle);

    if (MAX_MAPPING_OBJECT_USERS == GetNumOfActiveUsers(pAdminSector))
    {
        return nullptr;
    }

    bool bReaderFound = false;
    bool bWriterFound = false;
    size_t nBufferIdx = -1;

    for (size_t i = 0; i < MAX_MAPPING_OBJECT_USERS; ++i)
    {
        // if idx is marked as available, then memory actor address for it is not valid
        if (pAdminSector->userAvailablePool[i]) continue;

        std::cout << "Checking Actor " << i << std::endl;

        Actor* pActor = (Actor*)((byte*)m_buffHandle + GetAddressForActor(i));

        if (strSrcPath == pActor->GetSrcPath() && strDstPath == pActor->GetDstPath())
        {
            if (Role::Reader == pActor->GetRole())
            {
                std::cout << "Reader Found\n";
                bReaderFound = true;
                nBufferIdx = pActor->GetBufferId();
            }
            else if (Role::Writer == pActor->GetRole()) 
            {
                std::cout << "Writer Found\n";
                bWriterFound = true;
            }
        }
        else if (strSrcPath != pActor->GetSrcPath() && strDstPath == pActor->GetDstPath())
        {
            // destination path already taken
            return nullptr;
        }  // TODO: What if source same, but destination different
    }

    size_t nNewActorIdx = GetAvailableUserIdx(pAdminSector);
    Actor* pNewActor = nullptr;

    if (IDX_INVALID != nNewActorIdx)
    {
        if (bReaderFound && !bWriterFound)
        {
            pNewActor = new ((byte*)m_buffHandle + GetAddressForActor(nNewActorIdx))
                Actor(strSrcPath, strDstPath, nNewActorIdx, Role::Writer);
            BufferType* pBuffer = (BufferType*)((byte*)m_buffHandle + GetAddressForBuffer(nBufferIdx));

            pBuffer->Init(m_szFileName, false);
            pNewActor->Init(this, pBuffer, m_szFileName);

            pAdminSector->userAvailablePool[nNewActorIdx] = false;
        }
        else if (!bReaderFound)
        {
            nBufferIdx = GetAvailableBufferIdx(pAdminSector);

            if (IDX_INVALID != nBufferIdx)
            {
                pNewActor = new ((byte*)m_buffHandle + GetAddressForActor(nNewActorIdx))
                    Actor(strSrcPath, strDstPath, nNewActorIdx, Role::Reader);
                BufferType* pBuffer = new ((byte*)m_buffHandle + GetAddressForBuffer(nBufferIdx)) BufferType(nBufferIdx);
                
                pBuffer->Init(m_szFileName, true);
                pNewActor->Init(this, pBuffer, m_szFileName);

                pAdminSector->userAvailablePool[nNewActorIdx] = false;
                pAdminSector->bufferAvailablePool[nBufferIdx] = false;
            }
        }  // if bReaderFound and bWriterFound do nothing
    }

    return pNewActor;
}

Actor* Administrator::GetActorPtr(size_t nActorId) const
{
    IPCScopedLock lk(&m_mutex);

    Actor* pActor = (Actor*)((byte*)m_buffHandle + GetAddressForActor(nActorId));

    return pActor;
}

BufferType* Administrator::GetBufferPtr(size_t nBufferId) const
{
    IPCScopedLock lk(&m_mutex);

    BufferType* pBuffer = (BufferType*)((byte*)m_buffHandle + GetAddressForBuffer(nBufferId));

    return pBuffer;
}

size_t Administrator::GetNumOfActiveUsers(FileAdminSector* pAdminSector)
{
    if (nullptr == pAdminSector)
        return 0;

    size_t nResult = 0;

    for (size_t i = 0; i < pAdminSector->userAvailablePool.size(); ++i)
        if (!pAdminSector->userAvailablePool[i]) nResult++;

    return nResult;
}

size_t Administrator::GetAvailableUserIdx(FileAdminSector* pAdminSector)
{
    if (nullptr == pAdminSector)
        return IDX_INVALID;

    size_t nResult = IDX_INVALID;

    for (size_t i = 0; i < pAdminSector->userAvailablePool.size(); ++i)
    {
        if (pAdminSector->userAvailablePool[i])
        {
            nResult = i;
            break;
        }
    }

    return nResult;
}

size_t Administrator::GetAddressForActor(size_t nUserIdx)
{
    return sizeof(FileAdminSector) + nUserIdx * sizeof(Actor);
}

size_t Administrator::GetAvailableBufferIdx(FileAdminSector* pAdminSector)
{
    if (nullptr == pAdminSector)
        return IDX_INVALID;

    size_t nResult = IDX_INVALID;

    for (size_t i = 0; i < pAdminSector->bufferAvailablePool.size(); ++i)
    {
        if (pAdminSector->bufferAvailablePool[i])
        {
            nResult = i;
            break;
        }
    }

    return nResult;
}

size_t Administrator::GetAddressForBuffer(size_t nBufferIdx)
{
    return sizeof(FileAdminSector) + MAX_MAPPING_OBJECT_USERS * sizeof(Actor)
        + nBufferIdx * sizeof(BufferType);
}
