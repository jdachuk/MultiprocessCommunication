#include "ActorData.h"

ActorData::ActorData(const std::string& srcPath, const std::string& dstPath, const std::string& mapFile)
    : SrcPath(srcPath), DstPath(dstPath), MapFile(mapFile)
{}

bool operator==(const ActorData& lhs, const ActorData& rhs)
{
    return
        lhs.SrcPath == rhs.SrcPath &&
        lhs.DstPath == rhs.DstPath &&
        lhs.MapFile == rhs.MapFile;
}

ActorAdminMapFile::ActorAdminMapFile()
    : WriterFound(false)
{}
