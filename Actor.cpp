#include "Actor.h"

#include "FixedSizeQueue.h"
#include "IPCEvent.h"
#include "IPCMutex.h"
#include "IPCMap.h"
#include "IPCSafeQueueManager.h"

#include <fstream>
#include <mutex>

Actor::Actor(const ActorData& data)
	: m_ActorData(data)
	, m_MapFile(nullptr)
	, m_WriterFoundEvent(nullptr)
	, m_ReaderFinishedEvent(nullptr)
	, m_QueueManager(nullptr)
{
	const std::size_t actorHash = std::hash<ActorData>{}(m_ActorData);

	char fileName[MAX_PATH];
	char writerFoundEvent[MAX_PATH];
	char readerFinishedEvent[MAX_PATH];
	sprintf_s(fileName, "ActorMap_%s", std::to_string(actorHash).c_str());
	sprintf_s(writerFoundEvent, "WriterFoundEvent_%s", std::to_string(actorHash).c_str());
	sprintf_s(readerFinishedEvent, "ReaderFinishedEvent_%s", std::to_string(actorHash).c_str());

	m_MapFile = std::make_unique<IPCMap>(sizeof(FixedSizeQueue<ChunkInfoType, MAX_CHUNKS>), fileName);
	m_WriterFoundEvent = std::make_unique<IPCEvent>(writerFoundEvent);
	m_ReaderFinishedEvent = std::make_unique<IPCEvent>(readerFinishedEvent);
	m_QueueManager = std::make_unique<IPCSafeQueueManager<ChunkInfoType, MAX_CHUNKS>>(
		static_cast<FixedSizeQueue<ChunkInfoType, MAX_CHUNKS>*>(m_MapFile->GetHandle()), m_ActorData);
}

void Actor::Start()
{
	if (m_MapFile->IsCreated())
	{
		ReadFile();
	}
	else
	{
		WriteFile();
	}
}

void Actor::ReadFile()
{
	if (WAIT_TIMEOUT == m_WriterFoundEvent->Wait())
	{
		return;
	}

	std::ifstream input(m_ActorData.SrcPath, std::ios::binary);

	auto rdBuf = input.rdbuf();
	size_t nSize = 0;

	ChunkType chunk;

	while ((nSize = rdBuf->sgetn(chunk.data(), chunk.size())) && nSize)
	{
		m_QueueManager->Push({ chunk, nSize });
	}

	m_ReaderFinishedEvent->Notify();
}

void Actor::WriteFile()
{
	m_WriterFoundEvent->Notify();

	std::ofstream output(m_ActorData.DstPath, std::ios::binary | std::ios::trunc);

	bool readerFinished = false;
	do
	{
		auto buffData = m_QueueManager->Pop();
		output.write(buffData.first.data(), buffData.second);
		const auto waitResult = m_ReaderFinishedEvent->Wait(0);
		readerFinished |= WAIT_OBJECT_0 == waitResult;
	} while (!readerFinished || !m_QueueManager->Empty());
}
