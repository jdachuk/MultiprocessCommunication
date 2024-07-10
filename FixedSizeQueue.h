#pragma once

#include <array>

template<typename T, size_t N>
class FixedSizeQueue
{
public:
	FixedSizeQueue();

	bool Empty() const;
	bool Full() const;

	void Push(T&& item);
	T Pop();

private:
	size_t Size() const;
	size_t FirstIdx() const;
	size_t LastIdx() const;

	std::array<T, N> m_Queue;
	size_t m_Size;
	size_t m_Deletions;
};

#include "FixedSizeQueue.inl"
