#pragma once

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string_view>
#include <filesystem>
#include <sys/stat.h>

// simple error message function
inline void throw_error(const std::string& msg) {
	throw std::runtime_error(msg);
}

// throws errno, accepts additional context to be printed if needed
inline void throw_errno(const std::string& context = "") {
	std::string msg = context.empty() ? std::strerror(errno) : context + ": " + std::strerror(errno);
	throw std::runtime_error(msg);
}
