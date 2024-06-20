#include <array>
#include <iostream>
#include <filesystem>
#include <shared_mutex>
#include <fstream>

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#include "Actor.h"
#include "Administrator.h"


int main(int argc, char* argv[])
{
    //if (argc < 4)
    //{
    //    std::cout << "usage: \n"
    //        << "\t multiprocesscopytool.exe source destination mapping_object_name\n";
    //    return -1;
    //}

    //const std::string strSrcPath(argv[1]);
    //const std::string strDstPath(argv[2]);
    //const std::string strMapp(argv[3]);
    const std::string strSrcPath("C:\\Users\\Yosyp_Dachuk\\source\\repos\\C++ Mentoring Program\\MultiprocessCommunication\\build\\Debug\\x64\\src.txt");
    const std::string strDstPath("C:\\Users\\Yosyp_Dachuk\\source\\repos\\C++ Mentoring Program\\MultiprocessCommunication\\build\\Debug\\x64\\dst.txt");
    const std::string strMapp("Mapp");

    if (!std::filesystem::exists(strSrcPath))
    {
        std::cout << "Source file doesn't found\n";
        return ERROR_SRC;
    }

    Administrator admin;

    if (!admin.Init(strMapp.c_str())) std::cout << "Failed to init administrator\n";
    auto pActor = admin.CreateActor(strSrcPath.c_str(), strDstPath.c_str());
    if (nullptr != pActor) pActor->Start();

    return 0;
}