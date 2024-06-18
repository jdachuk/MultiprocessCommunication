#include "Actor.h"

#include "Buffer.h"

#include <array>
#include <fstream>

#include <iostream>

template<typename T, size_t N>
Actor<T, N>::Actor(const char* strSrcPath, const char* strDstPath, size_t nId, Role eRole)
	: m_nId(nId)
	, m_nBufferId(IDX_INVALID)
	, m_eRole(eRole)
	, m_bFinished(false)
	, m_pAdministrator(nullptr)
	, m_pBuffer(nullptr)
{
	memcpy(m_strSrcPath, strSrcPath, MAX_PATH_LENGTH);
	memcpy(m_strDstPath, strDstPath, MAX_PATH_LENGTH);
}

template<typename T, size_t N>
void Actor<T, N>::Start()
{
	if (Role::Reader == m_eRole)
		Read();
	else
		Write();
}

template<typename T, size_t N>
inline void Actor<T, N>::Init(Administrator<T, N>* pAdministrator)
{
	m_pAdministrator = pAdministrator;
	m_pBuffer = m_pAdministrator->GetBufferPtr(m_nBufferId);

	if (nullptr == m_pBuffer) return;

	if (Role::Reader == m_eRole)
		m_pBuffer->SetReaderId(m_nId);
	else
		m_pBuffer->SetWriterId(m_nId);
}

template<typename T, size_t N>
std::string Actor<T, N>::GetSrcPath() const
{
	return m_strSrcPath;
}

template<typename T, size_t N>
std::string Actor<T, N>::GetDstPath() const
{
	return m_strDstPath;
}

template<typename T, size_t N>
inline size_t Actor<T, N>::GetId() const
{
	return m_nId;
}

template<typename T, size_t N>
Role Actor<T, N>::GetRole() const
{
	return m_eRole;
}

template<typename T, size_t N>
bool Actor<T, N>::Finished() const
{
	return m_bFinished;
}

template<typename T, size_t N>
inline void Actor<T, N>::SetBufferId(size_t nBufferId)
{
	m_nBufferId = nBufferId;
}

template<typename T, size_t N>
size_t Actor<T, N>::GetBufferId() const
{
	return m_nBufferId;
}

template<typename T, size_t N>
void Actor<T, N>::Read()
{
	std::ifstream input(m_strSrcPath, std::ios::binary);

	auto rdBuf = input.rdbuf();
	size_t nSize = 0;

	while (!m_bFinished)
	{
		T vctBuffer;
		m_bFinished = !((nSize = rdBuf->sgetn(vctBuffer.data(), vctBuffer.size())) && nSize);
		m_pBuffer->Push(std::move(vctBuffer));
	}
}

template<typename T, size_t N>
void Actor<T, N>::Write()
{
	std::ofstream output(m_strDstPath, std::ios::binary | std::ios::trunc);

	size_t nReaderId = m_pBuffer->GetReaderId();
	const Actor<T, N>* pReader = m_pAdministrator->GetActorPtr(nReaderId);

	while (!pReader->Finished() || !m_pBuffer->Empty())
	{
		auto buffData = m_pBuffer->Pop();
		output.write(buffData.data(), buffData.size());
	}

	m_bFinished = true;
}
