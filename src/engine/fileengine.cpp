#include "fileengine.h"

#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <filesystem>

#include "utility.h"

namespace fs = std::filesystem;

// ! these functions does not validate these addresses.

// the primary copy engine for copying a single file around
// the IO_Process sent must have its source and destination values
// as individual files,

void copy_file_engine(IO_process& process) {
	std::string context = "In copy_file_engine()";
	process.open_files();

	if (process.m_same_device) {
		off_t src = 0, dst = 0;
		ssize_t remaining = process.m_source_info.st_size;
		while (remaining > 0) {
			ssize_t copied = copy_file_range(process.get_source_fd(), &src, process.get_destination_fd(), &dst, remaining, 0);
			if (copied < 0) {
				if (errno == EINTR)
					continue;
				throw_errno(context + ", for " + process.m_source.c_str() + " to " + process.m_destination.c_str());
			}
			remaining -= copied;
		}
	} else {
		char buffer[max_read_size];

		while (true) {
			ssize_t readptr;

			// read loop
			while (true) {
				readptr = read(process.get_source_fd(), buffer, max_read_size);

				if (readptr >= 0)	 // appropriate number of bytes have been read
					break;

				if (errno == EINTR)	// any interrupt that may have happened
					continue;

				throw_errno(context + ", during read, from:" + process.m_source.c_str() + ", to:" + process.m_destination.c_str());
			}

			if (readptr == 0)	 // end of file is reached
				break;

			ssize_t bytes_written = 0;
			// write loop
			while (bytes_written < readptr) {
				ssize_t result = write(process.get_destination_fd(), buffer + bytes_written, readptr - bytes_written);

				if (result > 0) {
					bytes_written += result;
					continue;
				}

				if (result == -1 && errno == EINTR)	 // regular interrupt recieved
					continue;

				throw_errno(context + ", during write, from:" + process.m_source.c_str() + ", to:" + process.m_destination.c_str());
			}
		}
	}

	process.finalize();	// commit the changes
}

// the primary copy engine for copying directories around.
// the IO_Process sent must have directories as source and destination.
void copy_directory_engine(IO_process& process, ThreadPool& pool) {
	std::string context = "In copy_directory_engine()";

	// iterate through all the directories and select current object as "src"
	for (const fs::directory_entry& src : fs::directory_iterator(process.m_source)) {
		// the current src's IO Process
		IO_process current;
		current.m_source = src.path();
		// real destination for current src
		current.m_destination = process.m_destination / src.path().filename();
		// since the child will have the same drive as destination parent
		current.m_same_device = process.m_same_device;

		// if src is a file, copy using file engine
		if (src.is_regular_file()) {
			// stat the source to obtain permissions
			if (stat(current.m_source.c_str(), &current.m_source_info) == -1)
				throw_errno(context + " , stat on: " + current.m_source.c_str());
			pool.add_process(current);


			// if src is a directory, make that directory at destination then copy it recursively
		} else if (src.is_directory()) {
			// stat the source to obtain permissions
			if (stat(current.m_source.c_str(), &current.m_source_info) == -1)
				throw_errno(context + " , stat on: " + current.m_source.c_str());
			if (mkdir(current.m_destination.c_str(), current.m_source_info.st_mode & 0777) != 0)
				throw_errno(context + ", mkdir on: " + current.m_destination.c_str());
			// copies the directory's contents
			copy_directory_engine(current, pool);


		} else if (src.is_symlink()) {
			// if its a symlink, simply skip
			std::cerr << "Skipping symlink: " << src.path() << '\n';
		}
	}
}

// calls rename() if same device
// otherwise falls to copy_file_engine and calls unlink()
void move_file_engine(IO_process& process) {
	static std::string context = "In move_file_engine()";
	if (process.m_same_device) {
		if (rename(process.m_source.c_str(), process.m_destination.c_str()) < 0)
			throw_errno(context + ", for: " + process.m_source.c_str());
	} else {
		copy_file_engine(process);
		if (unlink(process.m_source.c_str()) < 0)
			throw_errno(context + ", for: " + process.m_source.c_str());
	}
}

// calls rename() recursively if same device
// otherwise uses copy_directory_engine and rm rf's the source
void move_directory_engine(IO_process& process, ThreadPool& pool) {
	static std::string context = "In move_directory_engine()";
	if (process.m_same_device) {
		if (rename(process.m_source.c_str(), process.m_destination.c_str()) < 0)
			throw_errno(context + ", for: " + process.m_source.c_str());
	} else {
		// ! This does not call rm rf after copying, critical ! !
		// * It must be done in main() AFTER shutdown()
		copy_directory_engine(process, pool);
	}
}

uint64_t calculate_total_bytes(std::filesystem::path& path) {
	static std::string context = "In calculate_total_bytes()";
	uint64_t bytes{0};
	for (const fs::directory_entry& current : fs::recursive_directory_iterator(path)) {
		if (current.is_regular_file())
			bytes += current.file_size();
	}
	return bytes;
}