#include "Actor.h"

#include "Buffer.h"
#include "Administrator.h"

#include "IPCScopedLock.h"

#include <fstream>
#include <thread>

#include <iostream>

Actor::Actor(const char* strSrcPath, const char* strDstPath, size_t nId, Role eRole)
	: m_nId(nId)
	, m_nBufferId(IDX_INVALID)
	, m_eRole(eRole)
	, m_bFinished(false)
	, m_pAdministrator(nullptr)
	, m_pBuffer(nullptr)
{
	memset(m_strSrcPath, 0, MAX_PATH_LENGTH);
	memset(m_strDstPath, 0, MAX_PATH_LENGTH);
	memcpy(m_strSrcPath, strSrcPath, MAX_PATH_LENGTH);
	memcpy(m_strDstPath, strDstPath, MAX_PATH_LENGTH);
}

void Actor::Init(Administrator* pAdministrator, BufferType* pBuffer, const char* szFileName)
{

	char sMutexName[256];

	m_pAdministrator = pAdministrator;
	m_pBuffer = pBuffer;

	if (nullptr == m_pBuffer) return;

	m_nBufferId = m_pBuffer->GetId();

	sprintf_s(sMutexName, sizeof(sMutexName) / sizeof(char), "ActorCondition_%s_%zu", szFileName, m_nBufferId);
	m_condition.Init(sMutexName, Role::Writer == m_eRole);

	if (Role::Reader == m_eRole)
		m_pBuffer->SetReaderId(m_nId);
	else
		m_pBuffer->SetWriterId(m_nId);
}

void Actor::Start()
{
	if (Role::Reader == m_eRole)
		Read();
	else
		Write();
}

std::string Actor::GetSrcPath() const
{
	return m_strSrcPath;
}

std::string Actor::GetDstPath() const
{
	return m_strDstPath;
}

size_t Actor::GetId() const
{
	return m_nId;
}

Role Actor::GetRole() const
{
	return m_eRole;
}

bool Actor::Finished() const
{
	return m_bFinished;
}

bool Actor::Ready() const
{
	return m_bReady;
}

void Actor::SetBufferId(size_t nBufferId)
{
	m_nBufferId = nBufferId;
}

size_t Actor::GetBufferId() const
{
	return m_nBufferId;
}

void Actor::Read()
{
	std::cout << "Read\n";
	std::ifstream input(m_strSrcPath, std::ios::binary);

	auto rdBuf = input.rdbuf();
	size_t nSize = 0;

	ChunkType chunk;

	DWORD rt = m_condition.Wait(5000);

	if (WAIT_TIMEOUT == rt)
	{
		m_bFinished = true;
		std::cout << "Timed Out!\n";
		return;
	}

	while (!m_bFinished)
	{
		m_bFinished = !((nSize = rdBuf->sgetn(chunk.data(), chunk.size())) && nSize);
		std::cout << "Read " << chunk.data() << std::endl;
		m_pBuffer->Push({chunk, nSize});
	}
	std::cout << "Finished Read\n";
}

void Actor::Write()
{
	std::ofstream output(m_strDstPath, std::ios::binary | std::ios::trunc);

	size_t nReaderId = m_pBuffer->GetReaderId();
	Actor* pReader = m_pAdministrator->GetActorPtr(nReaderId);

	m_condition.NotifyOne();
	std::cout << "Write\n";

	while (!(pReader->Finished() && m_pBuffer->Empty()))
	{
		auto buffData = m_pBuffer->Pop();
		std::cout << "Write " << buffData.first.data() << std::endl;
		output.write(buffData.first.data(), buffData.second);
	}
	std::cout << "Finished Write\n";
	m_bFinished = true;
}
