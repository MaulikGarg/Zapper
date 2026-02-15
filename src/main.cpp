#include <filesystem>
#include <iostream>
#include <string>

#include "copyengine.h"
#include "validator.h"

namespace fs = std::filesystem;

void get_source(fs::path& source_path);
void get_destination(fs::path& destination_path);

int main() {
	try {
		fs::path source, destination;
		fs::file_status source_info, destination_info;

		std::cout << "File Zap 0.01\n";

		get_source(source);
		resolve_source(source, source_info);
		get_destination(destination);

		if (fs::is_regular_file(source_info)) {
			resolve_destination_file(source, destination, source_info, destination_info);
			copy_file_engine(source, destination);
		} else if (fs::is_directory(source_info)) {
			resolve_destination_directory_root(source, destination, destination_info);
		} else
			throw std::runtime_error("Unsupported format.");

		std::cout << "File Copy success. Exiting...\n";
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

void get_destination(fs::path& destination_path) {
	std::string destination_inp;
	std::cout << "Enter the destination file path: ";
	getline(std::cin, destination_inp);
	destination_path = destination_inp;
}
