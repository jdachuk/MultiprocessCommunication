#pragma once

#include "ActorData.h"
#include "FixedSizeQueue.h"

#include <memory>

class IPCEvent;
class IPCMutex;

template <typename T, int N>
class IPCSafeQueueManager
{
public:
	IPCSafeQueueManager(FixedSizeQueue<T, N>* queue, const ActorData& data);

	bool Empty() const;
	bool Full() const;

	void Push(T&& item);
	T Pop();

private:
	FixedSizeQueue<T, N>* m_Queue;
	std::unique_ptr<IPCMutex> m_Mutex;
	std::unique_ptr<IPCEvent> m_ReadEvent;
	std::unique_ptr<IPCEvent> m_WriteEvent;
};

#include "IPCSafeQueueManager.inl"
