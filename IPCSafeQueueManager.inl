#include "IPCSafeQueueManager.h"

#include "ActorData.h"
#include "IPCEvent.h"
#include "IPCMutex.h"

#include <mutex>

template<typename T, int N>
IPCSafeQueueManager<T, N>::IPCSafeQueueManager(FixedSizeQueue<T, N>* queue, const ActorData& data)
	: m_Queue(queue)
	, m_Mutex(nullptr)
	, m_ReadEvent(nullptr)
	, m_WriteEvent(nullptr)
{
	const std::size_t actorHash = std::hash<ActorData>{}(data);

	char mutexName[MAX_PATH];
	char readEventName[MAX_PATH];
	char writeEventName[MAX_PATH];
	sprintf_s(mutexName, "QueueMutex_%s", std::to_string(actorHash).c_str());
	sprintf_s(readEventName, "QueueReadEvent_%s", std::to_string(actorHash).c_str());
	sprintf_s(writeEventName, "QueueWriteEvent_%s", std::to_string(actorHash).c_str());

	m_Mutex = std::make_unique<IPCMutex>(mutexName);
	m_ReadEvent = std::make_unique<IPCEvent>(readEventName);
	m_WriteEvent = std::make_unique<IPCEvent>(writeEventName);
}

template<typename T, int N>
inline bool IPCSafeQueueManager<T, N>::Empty() const
{
	std::scoped_lock<IPCMutex> lk(*m_Mutex.get());
	return m_Queue->Empty();
}

template<typename T, int N>
inline bool IPCSafeQueueManager<T, N>::Full() const
{
	std::scoped_lock<IPCMutex> lk(*m_Mutex.get());
	return m_Queue->Full();
}

template<typename T, int N>
inline void IPCSafeQueueManager<T, N>::Push(T&& item)
{
	if (Full()) m_WriteEvent->Wait();

	std::scoped_lock<IPCMutex> lk(*m_Mutex.get());
	m_Queue->Push(std::move(item));

	m_ReadEvent->Notify();
}

template<typename T, int N>
inline T IPCSafeQueueManager<T, N>::Pop()
{
	if (Empty()) m_ReadEvent->Wait();

	std::scoped_lock<IPCMutex> lk(*m_Mutex.get());
	T item{ std::move(m_Queue->Pop()) };

	m_WriteEvent->Notify();

	return item;
}
