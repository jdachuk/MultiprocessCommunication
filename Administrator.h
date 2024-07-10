#pragma once

#include "ActorData.h"

#include <Windows.h>

#include <string>
#include <memory>

class Actor;
class IPCMap;
class IPCMutex;

class Administrator
{
public:
	Administrator(const std::string& srcPath, const std::string& dstPath, const std::string& mapFile);

	std::unique_ptr<Actor> CreateActor();

private:
	const ActorData m_ActorData;
	std::unique_ptr<IPCMutex> m_Mutex;
	std::unique_ptr<IPCMap> m_MapFile;
};

