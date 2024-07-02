#include "Actor.h"
#include "Administrator.h"
#include "IPCEvent.h"
#include "IPCMap.h"
#include "IPCMutex.h"
#include "IPCSafeQueueManager.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        std::cout << "usage: \n"
            << "\t multiprocesscopytool.exe source destination mapping_object_name\n";
        return -1;
    }

    const std::string srcPath(argv[1]);
    const std::string dstPath(argv[2]);
    const std::string mapName(argv[3]);

    auto admin = std::make_unique<Administrator>(srcPath, dstPath, mapName);
    auto actor = admin->CreateActor();

    actor->Start();
}
