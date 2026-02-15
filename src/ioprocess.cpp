#include "ioprocess.h"

// reads errno to the console and terminates the application
auto trigger_errno_crash = []() {
	throw std::runtime_error(std::strerror(errno));
};

void IO_process::cleanup(){
    if (source_fd >= 0) {
        close(source_fd);
        source_fd = -1;
    }
    if (destination_fd >= 0) {
        close(destination_fd);
        destination_fd = -1;
    }
}
