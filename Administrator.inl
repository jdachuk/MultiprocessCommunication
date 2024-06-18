#include "Actor.h"
#include "Buffer.h"
#include "Administrator.h"

#include "IPCMutex.h"
#include "IPCScopedLock.h"

#define FILE_SIZE(T, N) sizeof(FileAdminSector) + MAX_MAPPING_OBJECT_USERS * sizeof(Actor<T, N>) + MAX_MAPPING_BUFFERS * sizeof(Buffer<T, N>)

template<typename T, size_t N>
bool Administrator<T, N>::Init(const char* szFileName)
{
    char sMutexName[256];
    bool bFileExisted = true;

    sprintf_s(sMutexName, sizeof(sMutexName) / sizeof(char), "Mutex_%s", szFileName);
    m_mutex.Init(sMutexName);

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
            FILE_SIZE(T, N),         // maximum object size (low-order DWORD)
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
        FILE_SIZE(T, N));

    if (m_buffHandle == NULL)
    {
        return false;
    }

    if (!bFileExisted)
    {
        IPCScopedLock lk(m_mutex);

        FileAdminSector* pAdminSector = new ((byte*)m_buffHandle) FileAdminSector();

        for (size_t i = 0; i < pAdminSector->userAvailablePool.size(); ++i)
            pAdminSector->userAvailablePool[i] = true;
        for (size_t i = 0; i < pAdminSector->bufferAvailablePool.size(); ++i)
            pAdminSector->bufferAvailablePool[i] = true;
    }

    m_bInitialized = true;

    return true;
}

template<typename T, size_t N>
void Administrator<T, N>::Run()
{
    if (!m_bInitialized)
        return;

    std::array<std::pair<bool, bool>, MAX_MAPPING_BUFFERS> arrBufferUsersFinished;
    for (size_t i = 0; i < arrBufferUsersFinished.size(); ++i)
    {
        arrBufferUsersFinished[i].first = true;
        arrBufferUsersFinished[i].second = true;
    }

    m_bRunning = true;
    do
    {
        std::cout << "Checking for active users\n";

        IPCScopedLock lk(m_mutex);

        FileAdminSector* pAdminSector = (FileAdminSector*)((byte*)m_buffHandle);

        bool bIsAnyUsersWorking = false;

        for (size_t i = 0; i < MAX_MAPPING_OBJECT_USERS; ++i)
        {
            // if idx is marked as available, then memory actor address for it is not valid
            if (pAdminSector->userAvailablePool[i]) continue;

            Actor<T, N>* pActor = (Actor<T, N>*)((byte*)m_buffHandle + GetAddressForActor(i));

            const size_t nBufferIdx = pActor->GetBufferId();
            const bool bFinished = pActor->Finished();

            if (Role::Reader == pActor->GetRole())
                arrBufferUsersFinished[nBufferIdx].first = bFinished;
            else
                arrBufferUsersFinished[nBufferIdx].second = bFinished;

            if (bFinished)
            {
                pActor->~Actor();
                pAdminSector->userAvailablePool[i] = true;
            }
            else bIsAnyUsersWorking = true;
        }

        for (size_t i = 0; i < MAX_MAPPING_BUFFERS; ++i)
        {
            if (pAdminSector->bufferAvailablePool[i]) continue;

            Buffer<T, N>* pBuffer = (Buffer<T, N>*)((byte*)m_buffHandle + GetAddressForBuffer(i));

            if (arrBufferUsersFinished[i].first && arrBufferUsersFinished[i].second)
            {
                pBuffer->~Buffer();
                pAdminSector->bufferAvailablePool[i] = true;
            }
        }

        if (!bIsAnyUsersWorking)
        {
            m_bRunning = false;
        }

        Sleep(1000);
    } while (m_bRunning);
}

template<typename T, size_t N>
Actor<T, N>* Administrator<T, N>::CreateActor(const char strSrcPath[MAX_PATH_LENGTH], const char strDstPath[MAX_PATH_LENGTH])
{
    if (!m_bInitialized)
    {
        return nullptr;
    }

    IPCScopedLock lk(m_mutex);

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

        Actor<T, N>* pActor = (Actor<T, N>*)((byte*)m_buffHandle + GetAddressForActor(i));

        if (strSrcPath == pActor->GetSrcPath() && strDstPath == pActor->GetDstPath())
        {
            if (Role::Reader == pActor->GetRole())
            {
                bReaderFound = true;
                nBufferIdx = pActor->GetBufferId();
            }
            else if (Role::Writer == pActor->GetRole()) 
            {
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
    Actor<T, N>* pNewActor = nullptr;

    if (IDX_INVALID != nNewActorIdx)
    {
        if (bReaderFound && !bWriterFound)
        {
            Buffer<T, N>* pBuffer = (Buffer<T, N>*)((byte*)m_buffHandle + GetAddressForBuffer(nBufferIdx));
            pNewActor = new ((byte*)m_buffHandle + GetAddressForActor(nNewActorIdx))
                Actor<T, N>(strSrcPath, strDstPath, nNewActorIdx, Role::Writer);

            pBuffer->Init(m_szFileName);
            pNewActor->SetBufferId(nBufferIdx);

            pAdminSector->userAvailablePool[nNewActorIdx] = false;
        }
        else if (!bReaderFound)
        {
            nBufferIdx = GetAvailableBufferIdx(pAdminSector);

            if (IDX_INVALID != nBufferIdx)
            {
                Buffer<T, N>* pBuffer = new ((byte*)m_buffHandle + GetAddressForBuffer(nBufferIdx)) Buffer<T, N>(nBufferIdx);
                pNewActor = new ((byte*)m_buffHandle + GetAddressForActor(nNewActorIdx))
                    Actor<T, N>(strSrcPath, strDstPath, nNewActorIdx, Role::Reader);
                
                pBuffer->Init(m_szFileName);
                pNewActor->SetBufferId(nBufferIdx);

                pAdminSector->userAvailablePool[nNewActorIdx] = false;
                pAdminSector->bufferAvailablePool[nBufferIdx] = false;
            }
        }  // if bReaderFound and bWriterFound do nothing
    }

    return pNewActor;
}

template<typename T, size_t N>
inline Actor<T, N>* Administrator<T, N>::GetActorPtr(size_t nActorId) const
{
    IPCScopedLock lk(m_mutex);

    Actor<T, N>* pActor = (Actor<T, N>*)((byte*)m_buffHandle + GetAddressForActor(nActorId));

    return pActor;
}

template<typename T, size_t N>
inline Buffer<T, N>* Administrator<T, N>::GetBufferPtr(size_t nBufferId) const
{
    IPCScopedLock lk(m_mutex);

    Buffer<T, N>* pBuffer = (Buffer<T, N>*)((byte*)m_buffHandle + GetAddressForBuffer(nBufferId));

    return pBuffer;
}

template<typename T, size_t N>
size_t Administrator<T, N>::GetNumOfActiveUsers(FileAdminSector* pAdminSector)
{
    if (nullptr == pAdminSector)
        return 0;

    size_t nResult = 0;

    for (size_t i = 0; i < pAdminSector->userAvailablePool.size(); ++i)
        if (!pAdminSector->userAvailablePool[i]) nResult++;

    return nResult;
}

template<typename T, size_t N>
size_t Administrator<T, N>::GetAvailableUserIdx(FileAdminSector* pAdminSector)
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

template<typename T, size_t N>
size_t Administrator<T, N>::GetAddressForActor(size_t nUserIdx)
{
    return sizeof(FileAdminSector) + nUserIdx * sizeof(Actor<T, N>);
}

template<typename T, size_t N>
size_t Administrator<T, N>::GetAvailableBufferIdx(FileAdminSector* pAdminSector)
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

template<typename T, size_t N>
size_t Administrator<T, N>::GetAddressForBuffer(size_t nBufferIdx)
{
    return sizeof(FileAdminSector) + MAX_MAPPING_OBJECT_USERS * sizeof(Actor<T, N>)
        + nBufferIdx * sizeof(Buffer<T, N>);
}
