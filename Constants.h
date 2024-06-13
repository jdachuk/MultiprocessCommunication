#pragma once

#define MAX_MAPPING_OBJECT_USERS 4  // each of reader and writer are counting
#define MAX_MAPPING_BUFFERS (MAX_MAPPING_OBJECT_USERS / 2)
#define MAX_PATH_LENGTH 128
#define USER_BUFFER_SIZE 512
#define CHUNK_SIZE 32

#define ERROR_USAGE -1
#define ERROR_SRC -2
#define ERROR_DST -3

#define IDX_INVALID -1