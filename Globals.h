#pragma once

#include <array>
#include <utility>

#define MAX_MAPPING_OBJECT_USERS 4  // each of reader and writer are counting
#define MAX_MAPPING_BUFFERS (MAX_MAPPING_OBJECT_USERS / 2)
#define MAX_PATH_LENGTH 128
#define USER_BUFFER_SIZE 512
#define CHUNK_SIZE 64
#define MAX_CHUNKS 8

#define ERROR_USAGE -1
#define ERROR_SRC -2
#define ERROR_DST -3

#define IDX_INVALID -1

typedef std::array<char, CHUNK_SIZE> ChunkType;
typedef std::pair<ChunkType, size_t> ChunkInfoType;

template<typename T, size_t N>
class Buffer;

typedef Buffer<ChunkInfoType, MAX_CHUNKS> BufferType;
