#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <iostream>
#include <filesystem>
#include <filesystem>
#include <sys/stat.h>

namespace fs = std::filesystem;

// validates if the source path file exists and is regular.
// also fills the source_info struct provided
// if not, throws the general errno exception
void resolve_source_file(const fs::path& source, fs::file_status& source_info);

// validates the given destination path file for multiple checks, such as:
// whether the file exists, whether it is a duplicate for the source.
// additionally, the destination_info struct is filled
// if a check fails, an appropriate error is thrown
void resolve_destination_file(const fs::path& source, fs::path& destination, const fs::file_status& source_info, fs::file_status& destination_info);

#endif
