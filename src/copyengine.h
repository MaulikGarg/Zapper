#ifndef COPY_ENGINE_H
#define COPY_ENGINE_H

#include "validator.h"
#include "ioprocess.h"
#include <unistd.h>	//for close, write
#include <cerrno>	 //for errno
#include <cstring>

constexpr int permbitmask = 0644;	 // default perms
constexpr int max_read_size = 4096;	 // 4kb i/o size

// copies a single file, handles the file descriptors
void copy_file_engine(IO_process& process);


#endif
