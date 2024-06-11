#include <iostream>
#include <filesystem>

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#define BUF_SIZE 256
TCHAR szName[] = TEXT("CopyToolFileMappingObject");

#define MAX_MAPPING_OBJECT_USERS 8  // each of reader and writer are counting
#define MAX_MAPPING_BUFFERS MAX_MAPPING_OBJECT_USERS / 2
#define MAX_PATH_LENGTH 128
#define USER_BUFFER_SIZE 512

#define ERROR_USAGE -1
#define ERROR_SRC -2
#define ERROR_DST -3

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

struct MappingFileUserStructure
{
    unsigned int nUserIdx;
    int nPairIdx;
    unsigned int nBuffIdx;
    //UserStatus oStatus = UserStatus::Free;  // could be determinated from user resources indexes
    // nBuffIdx and nPairIdx not set - UserStatus::Free
    // nBuffIdx set and nPairIdx not set - UserStatus::Waiting
    // nBuffIdx and nPairIdx set - UserStatus::Working
    char strSrcFilePath[MAX_PATH_LENGTH];
    char strDstFilePath[MAX_PATH_LENGTH];
};

struct MappingFileAdminStructure
{
    unsigned int nActiveUsers;
    MappingFileUserStructure vctUsersInfo[MAX_MAPPING_OBJECT_USERS];
    MappingFileBuffStructure vctBuffers[MAX_MAPPING_BUFFERS];
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

DWORD try_CreateFileMapping(void** hMapFile, LPCTSTR szFileName)
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

DWORD try_MapViewOfFile(HANDLE hMapFile, void** pBuf)
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
    if (argc < 4)
    {
        std::cout << "Usage: \n"
            << "\t MultiprocessCopyTool.exe source destination mapping_object\n";
        return ERROR_USAGE;
    }

    const std::string strSrcPath(argv[1]);
    const std::string strDstPath(argv[2]);

    WCHAR sMappObj[512];
    swprintf(sMappObj, 512, L"%S", argv[3]);

    if (!std::filesystem::exists(strSrcPath))
    {
        std::cout << "Source file doesn't found\n";
        return ERROR_SRC;
    }

    if (std::filesystem::exists(strDstPath))
    {
        std::cout << "Destination file \"" << strDstPath << "\" already exists! Do you want to overwrite it?\n Y/N\n";
        char cOverwrite = 'n';
        std::cin >> cOverwrite;
        if (std::tolower(cOverwrite) != 'y')
            return ERROR_DST;
    }

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

        std::cout << "Reader " << pAdmin->nActiveUsers << std::endl;

        if (MAX_MAPPING_OBJECT_USERS < pAdmin->nActiveUsers)
        {
            pAdmin->nActiveUsers += 1;
        }

        (void)_getch();

        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
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

        MappingFileAdminStructure* pAdmin = new (pBuf) MappingFileAdminStructure();

        std::cout << "Writer " << pAdmin->nActiveUsers << std::endl;

        if (MAX_MAPPING_OBJECT_USERS > pAdmin->nActiveUsers)
        {
            pAdmin->nActiveUsers += 1;
        }

        (void)_getch();

        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
    }


    return 0;
}