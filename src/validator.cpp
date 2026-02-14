#include "validator.h"

void resolve_source_file(const fs::path& source, fs::file_status& source_info) {
	source_info = fs::status(source);

	if (!fs::exists(source_info))
		throw std::runtime_error("Source path does not exist.");

	if (!fs::is_regular_file(source_info))
		throw std::runtime_error("Expected a file in source path.");
}

void resolve_destination_file(const fs::path& source, fs::path& destination, const fs::file_status& source_info, fs::file_status& destination_info) {
	destination_info = fs::status(destination);

	if (fs::exists(destination_info)) {
		if (fs::is_directory(destination_info)) {	 // if the given path is a directory, correct it for a file
			destination = destination / source.filename();
			destination_info = fs::status(destination);
		}
		if (fs::exists(destination_info) && fs::equivalent(source, destination))  // if both src and dst are the same
			throw std::runtime_error("Source & Destination must be different");
	}

	else {
		fs::path parent = destination.parent_path().empty() ? "." : destination.parent_path();	 // get the appropriate parent name

		fs::file_status parent_status = fs::status(parent);

		if (!fs::exists(parent_status))
			throw std::runtime_error("Parent for destination doesn't exist.");

		if (!fs::is_directory(parent_status))
			throw std::runtime_error("Parent path is not a directory.");
	}
}
