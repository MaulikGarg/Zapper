#include "copyengine.h"
#include "threadpool.h"

namespace fs = std::filesystem;
// the primary copy engine for copying a single file around
// the IO_Process sent must have its source and destination values
// as individual files,
// ! this function does not validate these addresses.
void copy_file_engine(IO_process& process) {
	std::string context = "In copy_file_engine()";
	process.open_files();

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
	process.finalize();	// commit the changes
}

// the primary copy engine for copying directories around.
// the IO_Process sent must have directories as source and destination.
// ! this function does not validate sent addresses.
void copy_directory_engine(IO_process& process, ThreadPool& pool) {
	std::string context = "In copy_directory_engine()";

	// iterate through all the directories and select current object as "src"
	for (const fs::directory_entry& src : fs::directory_iterator(process.m_source)) {
		// the current src's IO Process
		IO_process current;
		current.m_source = src.path();
		// real destination for current src
		current.m_destination = process.m_destination / src.path().filename();
		// stat the source to obtain permissions
		if (stat(current.m_source.c_str(), &current.m_source_info) == -1)
			throw_errno(context + " , stat on: " + current.m_source.c_str());

		// if src is a file, copy using file engine
		if (src.is_regular_file()) {
			pool.add_process(current);
			// if src is a directory, make that directory at destination then copy it recursively
		} else if (src.is_directory()) {
			if (mkdir(current.m_destination.c_str(), current.m_source_info.st_mode & 0777) != 0)
				throw_errno(context + ", mkdir on: " + current.m_destination.c_str());
			copy_directory_engine(current, pool);	// copies the directory's contents
		}
	}
}
