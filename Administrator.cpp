#include "Administrator.h"

#include "Actor.h"
#include "IPCEvent.h"
#include "IPCMap.h"
#include "IPCMutex.h"
#include "IPCSafeQueueManager.h"

#include <mutex>

Administrator::Administrator(const std::string& srcPath, const std::string& dstPath, const std::string& mapFile)
	: m_ActorData(srcPath, dstPath, mapFile)
	, m_Mutex(nullptr)
	, m_MapFile(nullptr)
{
	const std::size_t actorHash = std::hash<ActorData>{}(m_ActorData);

	char fileName[MAX_PATH];
	sprintf_s(fileName, "AdminMap_%s", std::to_string(actorHash).c_str());

	m_Mutex = std::make_unique<IPCMutex>(fileName);

	std::scoped_lock<IPCMutex> lock(*m_Mutex.get());

	m_MapFile = std::make_unique<IPCMap>(sizeof(ActorAdminMapFile), fileName);

	ActorAdminMapFile* actorAdminMap = nullptr;

	if (!m_MapFile->IsCreated())
	{
		actorAdminMap = new (static_cast<byte*>(m_MapFile->GetHandle())) ActorAdminMapFile();
	}
	else
	{
		actorAdminMap = (ActorAdminMapFile*)m_MapFile->GetHandle();
	}

	if (actorAdminMap->WriterFound)
	{
		throw std::exception("Source and destination paths already processing!");
	}
}

std::unique_ptr<Actor> Administrator::CreateActor()
{
	std::scoped_lock<IPCMutex> lock(*m_Mutex.get());

	if (!m_MapFile->IsCreated())
	{
		static_cast<ActorAdminMapFile*>(m_MapFile->GetHandle())->WriterFound = true;
	}

	return std::make_unique<Actor>(m_ActorData);
}
