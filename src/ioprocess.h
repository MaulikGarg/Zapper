#pragma once

#include <fcntl.h>
#include <unistd.h>

#include "utility.h"

// the primary class responsible for managing file descriptors
// and the Byte Flux <3
class IO_process {
	// source file descriptor
	int m_source_fd{-1};
	// destination file descriptor
	int m_destination_fd{-1};
	// closes all file descriptors
	void cleanup();

  public:
	// if the source and destination are on the same device
	bool m_same_device{false};
	// the source path of the current IO Process
	std::filesystem::path m_source{};
	// the destination path of the current IO Process
	std::filesystem::path m_destination{};
	// the temporary file for this file
	std::filesystem::path m_temp{};
	struct stat m_source_info{};
	struct stat m_destination_info{};

	// primary destructor which calls cleanup() in order to clear FDs
	~IO_process() {
		cleanup();
	}

	// opens source and temp file and set their descriptors
	void open_files();
	// finalize the current operation with fsync, and renaming from temp
	void finalize();
	int get_source_fd() const {
		return m_source_fd;
	}
	int get_destination_fd() const {
		return m_destination_fd;
	}
};

