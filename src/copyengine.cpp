#include "copyengine.h"

void copy_file_engine(fs::path& source, fs::path& destination) {
	int source_fd = open(source.c_str(), O_RDONLY | O_CLOEXEC);
	if (source_fd < 0)
		trigger_errno_crash();

	int destination_fd = open(destination.c_str(), O_CREAT | O_TRUNC | O_WRONLY, permbitmask);
	if (destination_fd < 0)
		trigger_errno_crash();

	char buffer[max_read_size];

	while (true) {
		ssize_t readptr = read(source_fd, buffer, max_read_size);
		if (readptr == 0)
			break;  // eof
		if (readptr == -1)
			trigger_errno_crash();

		ssize_t bytes_written = 0;
		ssize_t bytes_left = readptr;
		while (bytes_left) {
			ssize_t result = write(destination_fd, buffer + bytes_written, bytes_left);
			if (result == -1)
				trigger_errno_crash();
			bytes_written += result;
			bytes_left -= result;
		}
	}

	fsync(destination_fd);
	close(source_fd);
	close(destination_fd);
}
