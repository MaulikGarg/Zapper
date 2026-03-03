#include <exception>
#include <iostream>
#include <vector>

#include "copyengine.h"
#include "ioprocess.h"
#include "threadpool.h"

enum whichpath {
	source,
	destination
};

void get_path(IO_process& process, whichpath path) {
	std::string input;
	if (path == source)
		std::cout << "Enter the source file path: ";
	if (path == destination)
		std::cout << "Enter the destination file path: ";

	getline(std::cin, input);
	auto start = input.find_first_not_of(" \t");
	auto end = input.find_last_not_of(" \t");
	input = (start == std::string::npos) ? "" : input.substr(start, end - start + 1);

	if (path == source) {
		process.m_source = input;
		process.m_source = process.m_source.lexically_normal();
	} else if (path == destination) {
		process.m_destination = input;
		process.m_destination = process.m_destination.lexically_normal();
	}
}

int main() {
	// the main process is the process which creates subprocesses if needed and
	// keeps everything maintained.
	IO_process mainprocess;
	try {
		std::cout << "File Zap 0.3\n";

		// get basic user input and validate if source is okay
		get_path(mainprocess, source);
		resolve_source(mainprocess);
		get_path(mainprocess, destination);

		// primary branching starts

		// if the source is a regular file
		if (S_ISREG(mainprocess.m_source_info.st_mode)) {
			resolve_destination_file(mainprocess);
			copy_file_engine(mainprocess);

			// if the source is a directory
		} else if (S_ISDIR(mainprocess.m_source_info.st_mode)) {
			ThreadPool mainpool;
			resolve_destination_directory_root(mainprocess);
			copy_directory_engine(mainprocess, mainpool);
			mainpool.shutdown();
			std::vector<std::exception_ptr> errors = std::move(mainpool.get_errors());
			for (auto err : errors) {
				if (err)
					try {
						std::rethrow_exception(err);
					} catch (std::exception e) {
						std::cout << e.what() << '\n';
					}
			}
		}
		// if the source is not a file or a directory
		else
			throw_error("Unsupported format.");

		std::cout << "File Copy success. Exiting...\n";
		return 0;
	} catch (std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
}
