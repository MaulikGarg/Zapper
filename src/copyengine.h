#ifndef COPY_ENGINE_H
#define COPY_ENGINE_H

#include "validator.h"
#include <fcntl.h>  //for open
#include <unistd.h>	//for close, write
#include <cerrno>	 //for errno
#include <cstring>

constexpr int permbitmask = 0644;	 // default perms
constexpr int max_read_size = 4096;	 // 4kb i/o size

// reads errno to the console and terminates the application
auto trigger_errno_crash = []() {
	throw std::runtime_error(std::strerror(errno));
};

// copies a single file, handles the file descriptors
void copy_file_engine(fs::path& source, fs::path& destination);


#endif
