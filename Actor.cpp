#include "Actor.h"

Actor::Actor(const std::string& strSrcPath, const std::string& strDstPath, size_t nBufferIdx, Role eRole)
	: m_strSrcPath(strSrcPath)
	, m_strDstPath(strDstPath)
	, m_nBufferIdx(nBufferIdx)
	, m_eRole(eRole)
{
}

std::string Actor::GetSrcPath() const
{
	return m_strSrcPath;
}

std::string Actor::GetDstPath() const
{
	return m_strDstPath;
}

size_t Actor::GetBufferIdx() const
{
	return m_nBufferIdx;
}

Actor::Role Actor::GetRole() const
{
	return m_eRole;
}
