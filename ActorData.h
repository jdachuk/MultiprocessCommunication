#pragma once

#include <string>

struct ActorData
{
	ActorData(const std::string& srcPath, const std::string& dstPath, const std::string& mapFile);

	std::string SrcPath;
	std::string DstPath;
	std::string MapFile;
};
bool operator==(const ActorData& lhs, const ActorData& rhs);

template<>
struct std::hash<ActorData>
{
	std::size_t operator()(const ActorData& data) const noexcept
	{
		std::size_t h1 = std::hash<std::string>{}(data.SrcPath);
		std::size_t h2 = std::hash<std::string>{}(data.DstPath);
		std::size_t h3 = std::hash<std::string>{}(data.MapFile);
		return (h1 ^ (h2 << 1)) ^ (h3 << 1);
	}
};

struct ActorAdminMapFile
{
	ActorAdminMapFile();

	bool WriterFound;
};

