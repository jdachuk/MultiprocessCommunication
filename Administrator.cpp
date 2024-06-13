#include "Administrator.h"

#include "Actor.h"
#include "Buffer.h"

#define FILE_SIZE sizeof(Administrator::FileAdminSector) \
    + MAX_MAPPING_OBJECT_USERS * sizeof(Actor) \
    + MAX_MAPPING_BUFFERS * sizeof(Buffer)

bool Administrator::Init(LPCTSTR szFileName)
{
    WCHAR sMutexName[512];
    bool bFileExisted = true;

    swprintf(sMutexName, 512, L"Mutex_%ls", szFileName);

    m_szFileName = szFileName;
    m_szMutexName = sMutexName;

    m_fileHandle = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,   // read/write access
        FALSE,                 // do not inherit the name
        m_szFileName);         // name of mapping object

    if (m_fileHandle == NULL && ERROR_FILE_NOT_FOUND == GetLastError())
    {
        bFileExisted = false;

        m_fileHandle = CreateFileMapping(
            INVALID_HANDLE_VALUE,    // use paging file
            NULL,                    // default security
            PAGE_READWRITE,          // read/write access
            0,                       // maximum object size (high-order DWORD)
            FILE_SIZE,               // maximum object size (low-order DWORD)
            m_szFileName);           // name of mapping object
    }
    
    if (m_fileHandle == NULL)
    {
        return false;
    }

    m_buffHandle = MapViewOfFile(m_fileHandle,   // handle to map object
        FILE_MAP_ALL_ACCESS,                     // read/write permission
        0,
        0,
        FILE_SIZE);

    if (m_buffHandle == NULL)
    {
        return false;
    }

    if (!bFileExisted)
    {
        m_mutexHandle = CreateMutex(NULL, TRUE, m_szMutexName);
        if (NULL == m_mutexHandle)
        {
            return false;
        }
        WaitForSingleObject(m_mutexHandle, INFINITE);

        FileAdminSector* pAdminSector = new ((byte*)m_buffHandle) FileAdminSector();

        for (size_t i = 0; i < pAdminSector->userAvailablePool.size(); ++i)
            pAdminSector->userAvailablePool[i] = true;
        for (size_t i = 0; i < pAdminSector->bufferAvailablePool.size(); ++i)
            pAdminSector->bufferAvailablePool[i] = true;

        ReleaseMutex(m_mutexHandle);
    }

	return true;
}

Actor* Administrator::CreateActor(const std::string& strSrcPath, const std::string& strDstPath)
{
    if (!m_bInitialized)
    {
        return nullptr;
    }

    m_mutexHandle = CreateMutex(NULL, TRUE, m_szMutexName);
    if (NULL == m_mutexHandle)
    {
        return nullptr;
    }
    WaitForSingleObject(m_mutexHandle, INFINITE);

    FileAdminSector* pAdminSector = (FileAdminSector*)((byte*)m_buffHandle);

    if (MAX_MAPPING_OBJECT_USERS == GetNumOfActiveUsers(pAdminSector))
    {
        ReleaseMutex(m_mutexHandle);
        return nullptr;
    }

    bool bReaderFound = false;
    bool bWriterFound = false;
    size_t nBufferIdx = -1;

    for (size_t i = 0; i < GetNumOfActiveUsers(pAdminSector); ++i)
    {
        // if idx is marked as available, then memory actor address for it is not valid
        if (pAdminSector->userAvailablePool[i]) continue;

        Actor* pActor = (Actor*)((byte*)m_buffHandle + GetAddressForActor(i));

        if (strSrcPath == pActor->GetSrcPath() && strDstPath == pActor->GetDstPath())
        {
            if (Actor::Role::Reader == pActor->GetRole()) 
            {
                bReaderFound = true;
                nBufferIdx = pActor->GetBufferIdx();
            }
            else if (Actor::Role::Writer == pActor->GetRole()) bWriterFound = true;
        }
        else if (strSrcPath != pActor->GetSrcPath() && strDstPath == pActor->GetDstPath())
        {
            // destination path already taken
            ReleaseMutex(m_mutexHandle);
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
                Actor(strSrcPath, strDstPath, nBufferIdx, Actor::Writer);
        }
        else if (!bReaderFound)
        {
            nBufferIdx = GetAvailableBufferIdx(pAdminSector);

            if (IDX_INVALID != nBufferIdx)
            {
                new ((byte*)m_buffHandle + GetAddressForBuffer(nBufferIdx)) Buffer();
                pNewActor = new ((byte*)m_buffHandle + GetAddressForActor(nNewActorIdx))
                    Actor(strSrcPath, strDstPath, nBufferIdx, Actor::Reader);
            }
        }  // if bReaderFound and bWriterFound do nothing

        pAdminSector->userAvailablePool[nNewActorIdx] = false;
    }

    ReleaseMutex(m_mutexHandle);

	return pNewActor;
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
        + nBufferIdx * sizeof(Buffer);
}
