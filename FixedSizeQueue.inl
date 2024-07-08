#include "FixedSizeQueue.h"

#include <assert.h>

template<typename T, size_t N>
FixedSizeQueue<T, N>::FixedSizeQueue()
	: m_Queue()
	, m_Size(0)
	, m_Deletions(0)
{}

template<typename T, size_t N>
bool FixedSizeQueue<T, N>::Empty() const
{
	return m_Size == m_Deletions;
}

template<typename T, size_t N>
bool FixedSizeQueue<T, N>::Full() const
{
	return N == Size();
}

template<typename T, size_t N>
void FixedSizeQueue<T, N>::Push(T&& item)
{
	assert(Size() <= N);

	m_Queue[LastIdx()] = item;
	++m_Size;
	m_Size %= SIZE_MAX;
}

template<typename T, size_t N>
T FixedSizeQueue<T, N>::Pop()
{
	T item = m_Queue[FirstIdx()];
	++m_Deletions;
	m_Deletions %= SIZE_MAX;

	assert(m_Size >= m_Deletions);

	return item;
}

template<typename T, size_t N>
size_t FixedSizeQueue<T, N>::Size() const
{
	return m_Size - m_Deletions;
}

template<typename T, size_t N>
size_t FixedSizeQueue<T, N>::FirstIdx() const
{
	return m_Deletions % N;
}

template<typename T, size_t N>
size_t FixedSizeQueue<T, N>::LastIdx() const
{
	return m_Size % N;
}
