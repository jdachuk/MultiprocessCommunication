#include <array>
#include <iostream>
#include <filesystem>
#include <shared_mutex>
#include <fstream>

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#include "Administrator.h"


int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        std::cout << "usage: \n"
            << "\t multiprocesscopytool.exe source destination mapping_object_name\n";
        return -1;
    }

    const std::string strSrcPath(argv[1]);
    const std::string strDstPath(argv[2]);

    if (!std::filesystem::exists(strSrcPath))
    {
        std::cout << "Source file doesn't found\n";
        return ERROR_SRC;
    }

    Administrator<std::array<char, 64>, 8> admin;

    if (!admin.Init(argv[3])) std::cout << "Failed to init administrator\n";
    auto pActor = admin.CreateActor(strSrcPath.c_str(), strDstPath.c_str());
    if (nullptr != pActor)
    {
        pActor->Init(&admin);
        pActor->Start();
    }
    //admin.Run();

    return 0;
}