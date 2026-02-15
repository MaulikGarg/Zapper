#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <filesystem>
#include "ioprocess.h"

// validates if the source path exists.
// also fills the source_info struct provided
// if not, throws an std exception
void resolve_source(IO_process& process);

// validates the given destination path file for multiple checks, such as:
// whether the file exists, whether it is a duplicate for the source.
// additionally, the destination_info struct is filled
// if a check fails, an appropriate error is thrown
void resolve_destination_file(IO_process& process);

// validates the given destination path for it to be a directory only.
// addtionally, the destination_info struct is filled
// if the given path is a file, an error is thrown.
// adjusts the destination path to be the appropriate target root.
void resolve_destination_directory_root(IO_process& process);
#endif
