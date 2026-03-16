#include <filesystem>
#include <iostream>

#include "fileengine.h"

enum e_whichpath {
	source,
	destination
};

void get_path(IO_process& process, e_whichpath path, std::string arg = "");

int main(int argc, char* argv[]) {
	// the main process is the process which creates subprocesses if needed and
	// keeps everything maintained.
	IO_process mainprocess;

	enum e_process {
		copy,
		move
	};

	e_process process;
	std::string flag{};

	try {
		std::cout << "File Zap 0.3\n";

		if (argc == 4) {
			get_path(mainprocess, source, argv[2]);
			resolve_source(mainprocess);
			get_path(mainprocess, destination, argv[3]);
			flag = argv[1];
		} else {
			// get basic user input and validate if source is okay
			get_path(mainprocess, source);
			resolve_source(mainprocess);
			get_path(mainprocess, destination);
			std::cout << "Enter the desired process(-c for copy, -m for move): ";
			std::cin >> flag;
		}

		if (flag == "-c")
			process = copy;
		else if (flag == "-m")
			process = move;
		else
			throw_error("Invalid flag: " + flag);
		// primary branching starts

		// if the source is a regular file
		if (S_ISREG(mainprocess.m_source_info.st_mode)) {
			resolve_destination_file(mainprocess);
			if (process == copy)
				copy_file_engine(mainprocess);
			else if (process == move)
				move_file_engine(mainprocess);

			// if the source is a directory
		} else if (S_ISDIR(mainprocess.m_source_info.st_mode)) {
			// the main threadpool where file IO Processes will be pushed
			ThreadPool mainpool;
			resolve_destination_directory_root(mainprocess);

			if (process == copy) {
				// set total bytes to be transferred
				mainpool.set_total_bytes(calculate_total_bytes(mainprocess.m_source));
				copy_directory_engine(mainprocess, mainpool);

			} else if (process == move) {
				// if the move is to a different device, calculates bytes to be moved
				if (!mainprocess.m_same_device)
					mainpool.set_total_bytes(calculate_total_bytes(mainprocess.m_source));
				move_directory_engine(mainprocess, mainpool);
			}

			// shut down all threads that mainpool may have opened
			mainpool.shutdown();

			// collect errors from mainpool into a local vector
			std::vector<std::exception_ptr> errors = std::move(mainpool.get_errors());
			// iterate over the error vector
			for (auto err : errors) {
				if (err)
					try {
						// rethrow the current exception so the relative catch block can print it
						std::rethrow_exception(err);
					} catch (std::exception& e) {
						std::cerr << e.what() << '\n';
					}
			}
			// if it was a move, safely destroy source since shutdown() is complete
			if (process == move) {
				if (errors.empty())
					std::filesystem::remove_all(mainprocess.m_source);
				else
					std::cerr << "Move incomplete, source not deleted due to errors.\n";
			}
		}
		// if the source is not a file or a directory
		else
			throw_error("Unsupported format.");

		std::cout << "Exiting...\n";
		return 0;
	} catch (std::exception& e) {
		// print out the current exception and leave
		std::cerr << e.what() << '\n';
		return 1;
	}
}

void get_path(IO_process& process, e_whichpath path, std::string arg) {
	std::string input;

	// if the CLI argument list is empty, print the appropriate prompt instead
	if (!arg.empty())
		input = arg;
	else {
		switch (path) {
			case source:
				std::cout << "Enter the source file path: ";
				break;
			case destination:
				std::cout << "Enter the destination file path: ";
				break;
		}
		getline(std::cin, input);
	}

	// remove all whitespace if any before and after the input
	auto start = input.find_first_not_of(" \t");
	auto end = input.find_last_not_of(" \t");
	input = (start == std::string::npos) ? "" : input.substr(start, end - start + 1);

	// set the source path and set it to be lexically normal
	if (path == source) {
		process.m_source = input;
		process.m_source = process.m_source.lexically_normal();

		// set the destination path and set it to be lexically normal
	} else if (path == destination) {
		process.m_destination = input;
		process.m_destination = process.m_destination.lexically_normal();
	}
}