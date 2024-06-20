#pragma once

#include "Globals.h"

#include "IPCConditionalVariable.h"

#include <string>

class Administrator;

enum Role
{
	Reader = 0,
	Writer,
};

class Actor
{
public:
	Actor(const char* strSrcPath, const char* strDstPath, size_t nId, Role eRole);
	~Actor() = default;

	void Init(Administrator* pAdministrator, BufferType* pBuffer, const char* szFileName);
	void Start();

	std::string GetSrcPath() const;
	std::string GetDstPath() const;
	size_t GetId() const;
	Role GetRole() const;
	bool Finished() const;
	bool Ready() const;

	void SetBufferId(size_t nBufferId);
	size_t GetBufferId() const;

private:
	void Read();
	void Write();

	char m_strSrcPath[MAX_PATH_LENGTH];
	char m_strDstPath[MAX_PATH_LENGTH];
	size_t m_nId;
	size_t m_nBufferId;
	Role m_eRole;
	bool m_bFinished;
	bool m_bReady;

	IPCConditionalVariable m_condition;

	Administrator* m_pAdministrator;
	BufferType* m_pBuffer;
};
