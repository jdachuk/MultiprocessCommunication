#pragma once

template<typename T, size_t N>
class Administrator;

template<typename T, size_t N>
class Buffer;

enum Role
{
	Reader = 0,
	Writer,
};

template<typename T, size_t N>
class Actor
{
public:
	Actor(const char* strSrcPath, const char* strDstPath, size_t nId, Role eRole);
	~Actor() = default;

	void Init(Administrator<T, N>* pAdministrator);
	void Start();

	std::string GetSrcPath() const;
	std::string GetDstPath() const;
	size_t GetId() const;
	Role GetRole() const;
	bool Finished() const;

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

	Administrator<T, N>* m_pAdministrator;
	Buffer<T, N>* m_pBuffer;
};

#include "Actor.inl"
