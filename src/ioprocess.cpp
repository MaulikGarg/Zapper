#include "ioprocess.h"

#include <fcntl.h>
#include <cerrno>
// closes both source and destination file descriptors of an
// IO_process
void IO_process::cleanup() {
	if (m_source_fd >= 0) {
		close(m_source_fd);
		m_source_fd = -1;
	}
	if (m_destination_fd >= 0) {
		close(m_destination_fd);
		m_destination_fd = -1;
	}
	if (!m_temp.empty()) {
		unlink(m_temp.c_str());
		m_temp = "";
	}
}

// commits any changes made in the current IO_process
// calls cleanup() internally
void IO_process::finalize() {
	std::string context = "In finalize()";
	// rename temp file to actual file
	if (rename(m_temp.c_str(), m_destination.c_str()) < 0)
		throw_errno(context + ", rename() failed for temp:" + m_temp.c_str());
	if (!m_temp.empty()) {
		m_temp = "";
	}
	cleanup();
}

// responsible for opening the files of the IO process
// ! This function uses O_TRUNC and therefore will overwrite any present destination file
void IO_process::open_files() {
	std::string context = "In open_files()";
	// open the source file descriptor with read only flag
	m_source_fd = open(m_source.c_str(), O_RDONLY | O_NOATIME);
	// if its a bad file descriptor because of permission error (O_NOATIME) try again without
	if(m_source_fd < 0 && errno == EPERM){
		m_source_fd = open(m_source.c_str(), O_RDONLY);
	}
	// if its still a bad file descriptor
	if (m_source_fd < 0) {
		throw_errno(context + ", failed to open source_fd for source: " + m_source.c_str());
	}

	// calculate the path for the temporary file
	m_temp = m_destination.parent_path() / ("." + m_destination.filename().string() + ".bf.tmp");
	// open the destination file descriptor with write only flag and creation
	m_destination_fd = open(m_temp.c_str(), O_CREAT | O_TRUNC | O_WRONLY, m_source_info.st_mode & 0777);
	// if its a bad file descriptor
	if (m_destination_fd < 0) {
		throw_errno(context + ", failed to open destination fd for destination:" + m_destination.c_str());
	}
}
