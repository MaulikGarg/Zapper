#include "validator.h"

namespace fs = std::filesystem;

void resolve_source(IO_process& process) {
	process.source_info = fs::status(process.source);
	if (!fs::exists(process.source_info))
		throw std::runtime_error("Source path does not exist.");
}

void resolve_destination_file(IO_process& process) {
	process.destination_info = fs::status(process.destination);

	if (fs::exists(process.destination_info)) {
		if (fs::is_directory(process.destination_info)) {	 // if the given path is a directory, correct it for a file
			process.destination = process.destination / process.source.filename();
			process.destination_info = fs::status(process.destination);
		}
		if (fs::exists(process.destination_info) && fs::equivalent(process.source, process.destination))  // if both src and dst are the same
			throw std::runtime_error("Source & Destination must be different");
	}

	else {
		fs::path parent = process.destination.parent_path().empty() ? "." : process.destination.parent_path();	 // get the appropriate parent name

		fs::file_status parent_status = fs::status(parent);

		if (!fs::exists(parent_status))
			throw std::runtime_error("Parent for destination doesn't exist.");

		if (!fs::is_directory(parent_status))
			throw std::runtime_error("Parent path is not a directory.");
	}
}

void resolve_destination_directory_root(IO_process& process){

}
