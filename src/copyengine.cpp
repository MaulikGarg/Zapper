#include "copyengine.h"

namespace fs = std::filesystem;

// reads errno to the console and terminates the application
auto trigger_errno_crash = []() {
	throw std::runtime_error(std::strerror(errno));
};

void copy_file_engine(IO_process& process) {
	process.open_files();

	char buffer[max_read_size];

	while (true) {
		ssize_t readptr = read(process.get_source_fd(), buffer, max_read_size);
		if (readptr == 0)
			break;  // eof
		if (readptr == -1)
			trigger_errno_crash();

		ssize_t bytes_written = 0;
		ssize_t bytes_left = readptr;
		while (bytes_left) {
			ssize_t result = write(process.get_destionation_fd(), buffer + bytes_written, bytes_left);
			if (result == -1)
				trigger_errno_crash();
			bytes_written += result;
			bytes_left -= result;
		}
	}
}
