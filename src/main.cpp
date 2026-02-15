#include <filesystem>
#include <iostream>
#include <string>

#include "copyengine.h"
#include "validator.h"
#include "ioprocess.h"

namespace fs = std::filesystem;

void get_source(fs::path& source_path);
void get_destination(fs::path& destination_path);

int main() {
	try {
		IO_process mainprocess; // every operation is a process in itself

		std::cout << "File Zap 0.01\n";

		get_source(mainprocess);
		resolve_source(mainprocess);
		get_destination(mainprocess);

		if (fs::is_regular_file(mainprocess.source_info)) {
			resolve_destination_file(mainprocess);
			copy_file_engine(mainprocess);
		} else if (fs::is_directory(mainprocess.source_info)) {
			resolve_destination_directory_root(mainprocess);
		} else
			throw std::runtime_error("Unsupported format.");

		std::cout << "File Copy success. Exiting...\n";
		return 0;
	} catch (std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
}

void get_source(IO_process& process) {
	std::string source_inp;
	std::cout << "Enter the source file path: ";
	getline(std::cin, source_inp);
	process.source = source_inp;
}

void get_destination(IO_process& process) {
	std::string destination_inp;
	std::cout << "Enter the destination file path: ";
	getline(std::cin, destination_inp);
	process.destination = destination_inp;
}
