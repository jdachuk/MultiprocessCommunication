#pragma once

#include <string>

class Actor
{
public:
	enum Role
	{
		Reader = 0,
		Writer,
	};

	Actor(const std::string& strSrcPath, const std::string& strDstPath, size_t nBufferIdx, Role eRole);

	std::string GetSrcPath() const;
	std::string GetDstPath() const;
	size_t GetBufferIdx() const;
	Role GetRole() const;

private:
	std::string m_strSrcPath;
	std::string m_strDstPath;
	size_t m_nBufferIdx;
	Role m_eRole;
};

