#include <array>
#include <iostream>
#include <filesystem>
#include <shared_mutex>
#include <fstream>

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#include "ThreadSafeQueue.h"

#define MAX_MAPPING_OBJECT_USERS 4  // each of reader and writer are counting
#define MAX_MAPPING_BUFFERS MAX_MAPPING_OBJECT_USERS / 2
#define MAX_PATH_LENGTH 128
#define USER_BUFFER_SIZE 512
#define CHUNK_SIZE 32

#define ERROR_USAGE -1
#define ERROR_SRC -2
#define ERROR_DST -3

#define IDX_INVALID -1

enum UserStatus
{
    Free = 0,
    Waiting,  // waiting for reader
    Working,  // both reader and writer 
};

struct MappingFileBuffStructure
{
    char vctBuff[USER_BUFFER_SIZE];
};

class MappingFileUserStructure
{
public:
    unsigned int nUserIdx;
    unsigned int nPairIdx = IDX_INVALID;
    unsigned int nBuffIdx = IDX_INVALID;
    //UserStatus oStatus = UserStatus::Free;  // could be determinated from user resources indexes
    // nBuffIdx and nPairIdx not set - UserStatus::Free
    // nBuffIdx set and nPairIdx not set - UserStatus::Waiting
    // nBuffIdx and nPairIdx set - UserStatus::Working
    std::string strSrcFilePath;
    //char strSrcFilePath[MAX_PATH_LENGTH];
    std::string strDstFilePath;
    //char strDstFilePath[MAX_PATH_LENGTH];
};

struct MappingFileBufferData
{
    bool completed;
    ThreadSafeQueue<std::pair<std::array<char, CHUNK_SIZE>, size_t>, USER_BUFFER_SIZE / CHUNK_SIZE> buffer;
};

struct MappingFileAdminStructure
{
    unsigned int nActiveUsers;
    MappingFileUserStructure vctUserInfo[MAX_MAPPING_OBJECT_USERS];
    MappingFileBufferData vctBufferData[MAX_MAPPING_BUFFERS];
};

#define FILE_SIZE sizeof(MappingFileAdminStructure)


DWORD try_OpenFileMapping(HANDLE* hMapFile, LPCTSTR szFileName)
{
    *hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,   // read/write access
        FALSE,                 // do not inherit the name
        szFileName);           // name of mapping object

    if (*hMapFile == NULL)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

DWORD try_CreateFileMapping(HANDLE* hMapFile, LPCTSTR szFileName)
{
    *hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,    // use paging file
        NULL,                    // default security
        PAGE_READWRITE,          // read/write access
        0,                       // maximum object size (high-order DWORD)
        FILE_SIZE,               // maximum object size (low-order DWORD)
        szFileName);             // name of mapping object

    if (*hMapFile == NULL)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

DWORD try_MapViewOfFile(HANDLE hMapFile, HANDLE* pBuf)
{
    *pBuf = MapViewOfFile(hMapFile,   // handle to map object
        FILE_MAP_ALL_ACCESS, // read/write permission
        0,
        0,
        FILE_SIZE);

    if (pBuf == NULL)
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}

int main(int argc, char* argv[])
{
    //if (argc < 4)
    //{
    //    std::cout << "Usage: \n"
    //        << "\t MultiprocessCopyTool.exe source destination mapping_object_name\n";
    //    return ERROR_USAGE;
    //}

    //const std::string strSrcPath(argv[1]);
    //const std::string strDstPath(argv[2]);

    WCHAR sMappObj[512];
    swprintf(sMappObj, 512, L"%S", "Mapp");
    //swprintf(sMappObj, 512, L"%S", argv[3]);
    WCHAR sMutexName[512];
    swprintf(sMutexName, 512, L"Mutex_%S", argv[3]);

    /*if (!std::filesystem::exists(strSrcPath))
    {
        std::cout << "Source file doesn't found\n";
        return ERROR_SRC;
    }*/

    HANDLE hMapFile;
    void* pBuf;
    DWORD uError = ERROR_SUCCESS;

    uError = try_OpenFileMapping(&hMapFile, sMappObj);

    if (ERROR_SUCCESS == uError)
    {
        uError = try_MapViewOfFile(hMapFile, &pBuf);

        if (ERROR_SUCCESS != uError)
        {
            _tprintf(TEXT("Could not map view of file (%d).\n"), uError);

            CloseHandle(hMapFile);

            return 1;
        }

        MappingFileAdminStructure* pAdmin = (MappingFileAdminStructure*)pBuf;

        //std::ofstream output(strDstPath.c_str(), std::ios::binary | std::ios::trunc);

        //while (!pAdmin->vctBufferData[0].completed || !pAdmin->vctBufferData[0].buffer.empty())
        //{
        //    if (!pAdmin->vctBufferData[0].completed && pAdmin->vctBufferData[0].buffer.empty()) continue;
        //    auto buffData = pAdmin->vctBufferData[0].buffer.popFront();
        //    output.write(buffData.first.data(), buffData.second);
        //}

        //std::cout << "Writing completed\n";

        std::cout << "Reader " << pAdmin->nActiveUsers << std::endl;

        if (MAX_MAPPING_OBJECT_USERS < pAdmin->nActiveUsers)
        {
            pAdmin->nActiveUsers += 1;
        }

        //if (!pAdmin->vctUserInfo[0].mutex.try_lock())
        //    std::cout << "Waiting" << std::endl;

        //(void)_getch();
        std::cout << "Waiting\n";
        HANDLE mutexHandle = CreateMutex(NULL, TRUE, sMutexName);
        WaitForSingleObject(mutexHandle, INFINITE);
        std::cout << "Finished\n";

        UnmapViewOfFile(pBuf);
    }
    else if (ERROR_FILE_NOT_FOUND == uError)
    {
        CloseHandle(hMapFile);

        uError = try_CreateFileMapping(&hMapFile, sMappObj);

        if (ERROR_SUCCESS != uError)
        {
            _tprintf(TEXT("Could not create file mapping object (%d).\n"), uError);
            return 1;
        }

        uError = try_MapViewOfFile(hMapFile, &pBuf);

        if (ERROR_SUCCESS != uError)
        {
            _tprintf(TEXT("Could not map view of file (%d).\n"), uError);

            CloseHandle(hMapFile);

            return 1;
        }

        //MappingFileAdminStructure* pAdmin = new (pBuf) MappingFileAdminStructure();
        MappingFileAdminStructure* pAdmin = (MappingFileAdminStructure*)pBuf;

        std::cout << "Writer " << pAdmin->nActiveUsers << std::endl;
        HANDLE mutexHandle = CreateMutex(NULL, TRUE, sMutexName);

        if (NULL == mutexHandle)
        {
            _tprintf(TEXT("Could not create mutex (%d).\n"), GetLastError());
            return 1;
        }

        if (MAX_MAPPING_OBJECT_USERS > pAdmin->nActiveUsers)
        {
            pAdmin->nActiveUsers += 1;
        }
        (void)_getch();
        //SignalObjectAndWait(mutexHandle, NULL, INFINITE, TRUE);
        ReleaseMutex(mutexHandle);

        //pAdmin->vctUserInfo[0].strSrcFilePath = strSrcPath;
        //pAdmin->vctUserInfo[0].strDstFilePath = strDstPath;

        //if (!std::filesystem::exists(strSrcPath))
        //{
        //    std::cout << "Source file doesn't found\n";
        //    return ERROR_SRC;
        //}

        //if (std::filesystem::exists(strDstPath))
        //{
        //    std::cout << "Destination file \"" << strDstPath << "\" already exists! Do you want to overwrite it?\n Y/N\n";
        //    char cOverwrite = 'n';
        //    std::cin >> cOverwrite;
        //    if (std::tolower(cOverwrite) != 'y')
        //        return ERROR_DST;
        //}

        //std::ifstream input(strSrcPath.c_str(), std::ios::binary);

        //std::array<char, CHUNK_SIZE> vctBuffer;

        //auto rdBuf = input.rdbuf();
        //size_t nSize = 0;
        //(void)_getch();

        //while ((nSize = rdBuf->sgetn(vctBuffer.data(), vctBuffer.size())) && nSize)
        //{
        //    pAdmin->vctBufferData[0].buffer.pushBack({vctBuffer, nSize});
        //}

        //pAdmin->vctBufferData[0].completed = true;

        //std::cout << "Reading completed\n";

        //pAdmin->vctUserInfo[0].mutex.lock();


        UnmapViewOfFile(pBuf);
    }

    CloseHandle(hMapFile);

    return 0;
}