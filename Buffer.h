#pragma once

#include <windows.h>

#include <array>

#include "IPCMutex.h"
#include "IPCSemaphore.h"
#include "IPCConditionalVariable.h"

template<typename T, size_t N>
class Buffer
{
public:
	Buffer(size_t nIdx);
	~Buffer() = default;

	void Init(const char* szFileName, bool bReading);

	size_t GetId() const;

	void SetReaderId(size_t nReaderId);
	size_t GetReaderId() const;

	void SetWriterId(size_t nWriterId);
	size_t GetWriterId() const;

	bool Empty() const;
	bool Full() const;

	void Push(T&& item);
	T Pop();

	void Clear();

private:
	size_t Size() const;
	size_t FirstIdx() const;
	size_t LastIdx() const;

	size_t m_nId;
	std::array<T, N> m_queue;
	size_t m_nSize;
	size_t m_nDeletions;

	size_t m_nReaderId;
	size_t m_nWriterId;

	mutable IPCMutex m_mutex;
	mutable IPCConditionalVariable m_cvRead;
	mutable IPCConditionalVariable m_cvWrite;
};

#include "Buffer.inl"
