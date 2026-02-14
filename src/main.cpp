#include <fcntl.h>  //for open
#include <sys/stat.h>
#include <unistd.h>	//for close, write

#include <cerrno>	 //for errno
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>

#include "validator.h"

namespace fs = std::filesystem;

constexpr int permbitmask = 0644;	 // default perms
constexpr int max_read_size = 4096;	 // 4kb i/o size

// reads errno to the console and terminates the application
auto trigger_errno_crash = []() {
	throw std::runtime_error(std::strerror(errno));
};

void get_source(fs::path& source_path);
void get_destination(fs::path& destination_path);
void copy_source_to_dest(int source_fd, int destination_fd);

int main() {
	try {
		std::cout << "File Zap 0.01\n";

		fs::path source;
		fs::file_status source_info;
		get_source(source);
		resolve_source_file(source, source_info);
		int source_fd = open(source.c_str(), O_RDONLY);
		if (source_fd < 0)
			trigger_errno_crash();
		
		fs::path destination;
		fs::file_status destination_info;
		get_destination(destination);
		resolve_destination_file(source, destination, source_info, destination_info);

		int destination_fd = open(destination.c_str(), O_CREAT | O_TRUNC | O_WRONLY, permbitmask);
		if (destination_fd < 0)
			trigger_errno_crash();

		copy_source_to_dest(source_fd, destination_fd);

		fsync(destination_fd);
		close(source_fd);
		close(destination_fd);

		std::cout << "File Copy success. Exiting...";
		return 0;
	} catch (std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
}

void get_source(fs::path& source_path) {
	std::string source_inp;
	std::cout << "Enter the source file path: ";
	getline(std::cin, source_inp);
	source_path = source_inp;
}

void get_destination(fs::path& destination_path){
	std::string destination_inp;
	std::cout << "Enter the destination file path: ";
	getline(std::cin, destination_inp);
	destination_path = destination_inp;
}

void copy_source_to_dest(int source_fd, int destination_fd) {
	char buffer[max_read_size];

	while (true) {
		ssize_t readptr = read(source_fd, buffer, max_read_size);
		if (readptr == 0)
			break;  // eof
		if (readptr == -1)
			throw std::runtime_error(std::strerror(errno));

		ssize_t bytes_written = 0;
		ssize_t bytes_left = readptr;
		while (bytes_left) {
			ssize_t result = write(destination_fd, buffer + bytes_written, bytes_left);
			if (result == -1)
				throw std::runtime_error(std::strerror(errno));
			bytes_written += result;
			bytes_left -= result;
		}
	}
}
