#include "Buffer.h"

#include "IPCMutex.h"
#include "IPCScopedLock.h"

#include <iostream>

template<typename T, size_t N>
Buffer<T, N>::Buffer(size_t nIdx)
	: m_nId(nIdx)
	, m_queue()
	, m_nSize(0)
	, m_nDeletions(0)
	, m_nReaderId(IDX_INVALID)
	, m_nWriterId(IDX_INVALID)
	, m_mutex()
	, m_cvRead()
	, m_cvWrite()
{
}

template<typename T, size_t N>
inline void Buffer<T, N>::Init(const char* szFileName, bool bReading)
{
	char sMutexName[256];

	sprintf_s(sMutexName, sizeof(sMutexName) / sizeof(char), "Local\\Mutex_%s_%zu", szFileName, m_nId);
	m_mutex.Init(sMutexName);

	sprintf_s(sMutexName, sizeof(sMutexName) / sizeof(char), "ConditionRead_%s_%zu", szFileName, m_nId);
	m_cvRead.Init(sMutexName, bReading);

	sprintf_s(sMutexName, sizeof(sMutexName) / sizeof(char), "ConditionWrite_%s_%zu", szFileName, m_nId);
	m_cvWrite.Init(sMutexName, !bReading);
}

template<typename T, size_t N>
size_t Buffer<T, N>::GetId() const
{
	return m_nId;
}

template<typename T, size_t N>
inline void Buffer<T, N>::SetReaderId(size_t nReaderId)
{
	m_nReaderId = nReaderId;
}

template<typename T, size_t N>
inline size_t Buffer<T, N>::GetReaderId() const
{
	return m_nReaderId;
}

template<typename T, size_t N>
inline void Buffer<T, N>::SetWriterId(size_t nWriterId)
{
	m_nWriterId = nWriterId;
}

template<typename T, size_t N>
inline size_t Buffer<T, N>::GetWriterId() const
{
	return m_nWriterId;
}

template<typename T, size_t N>
bool Buffer<T, N>::Empty() const
{
	IPCScopedLock lk(&m_mutex);

	return (m_nDeletions >= m_nSize);
}

template<typename T, size_t N>
bool Buffer<T, N>::Full() const
{
	IPCScopedLock lk(&m_mutex);

	return (N <= m_nSize - m_nDeletions);
}

template<typename T, size_t N>
void Buffer<T, N>::Push(T&& item)
{
	if (Full()) 
	{
		std::cout << "Push waiting for signal\n";
		m_cvWrite.Wait(1000);
	}
	std::cout << "Push got signal\n";
	IPCScopedLock lk(&m_mutex);
	//m_mutex.Lock();

	m_queue[LastIdx()] = item;
	m_nSize += 1;

	//m_mutex.Unlock();
	m_cvRead.NotifyOne();
}

template<typename T, size_t N>
T Buffer<T, N>::Pop()
{
	if (Empty()) 
	{
		std::cout << "Pop waiting for signal\n";
		m_cvRead.Wait(1000);
	}
	std::cout << "Pop got signal\n";
	IPCScopedLock lk(&m_mutex);
	//m_mutex.Lock();

	T item = m_queue[FirstIdx()];
	m_nDeletions += 1;

	//m_mutex.Unlock();
	m_cvWrite.NotifyOne();

	return item;
}

template<typename T, size_t N>
void Buffer<T, N>::Clear()
{
}

template<typename T, size_t N>
inline size_t Buffer<T, N>::Size() const
{
	IPCScopedLock lk(&m_mutex);

	return m_nSize - m_nDeletions;
}

template<typename T, size_t N>
inline size_t Buffer<T, N>::FirstIdx() const
{
	return m_nDeletions % N;
}

template<typename T, size_t N>
inline size_t Buffer<T, N>::LastIdx() const
{
	return (m_nSize) % N;
}
