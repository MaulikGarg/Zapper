#pragma once

#include <unistd.h>	//for close, write
#include "ioprocess.h"
#include "threadpool.h"
#include "utility.h"
#include "validator.h"
#include <cstdint>
#include <filesystem>
#include <iostream>
constexpr int max_read_size = 1'048'576;	// 1mb i/o size

// copies a single file, handles the file descriptors
void copy_file_engine(IO_process& process);

// copies an entire directory
// the destination directory root must exist
void copy_directory_engine(IO_process& process, ThreadPool& pool);

// moves a single file, if same st_dev, simply renames.
// otherwise, calls copy_file_engine then deletes source.
void move_file_engine(IO_process& process);

// moves an entire directory.
// if same st_dev, simply renames.
// otherwise, calls copy_directory_engine, main() is responsible for deleting source
void move_directory_engine(IO_process& process, ThreadPool& pool);

// walks a directory tree and returns the total size of its contents in bytes
uint64_t calculate_total_bytes(std::filesystem::path& path);