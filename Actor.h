#pragma once

#include "ActorData.h"
#include "Globals.h"

#include <memory>

class IPCEvent;
class IPCMutex;
class IPCMap;

template<typename T, int N>
class IPCSafeQueueManager;

class Actor
{
public:
	Actor(const ActorData& data);

	void Start();

private:
	void ReadFile();
	void WriteFile();

private:
	const ActorData m_ActorData;
	std::unique_ptr<IPCMap> m_MapFile;
	std::unique_ptr<IPCEvent> m_WriterFoundEvent;
	std::unique_ptr<IPCEvent> m_ReaderFinishedEvent;
	std::unique_ptr<IPCSafeQueueManager<ChunkInfoType, MAX_CHUNKS>> m_QueueManager;
};

