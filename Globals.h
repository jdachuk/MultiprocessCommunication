#pragma once

#include <array>
#include <utility>

#define CHUNK_SIZE 64
#define MAX_CHUNKS 8

typedef std::array<char, CHUNK_SIZE> ChunkType;
typedef std::pair<ChunkType, size_t> ChunkInfoType;
