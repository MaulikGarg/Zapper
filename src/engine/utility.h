#pragma once

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string_view>
#include <filesystem>
#include <sys/stat.h>
#include <atomic>

// simple error message function
inline void throw_error(const std::string& msg) {
	throw std::runtime_error(msg);
}

// throws errno, accepts additional context to be printed if needed
inline void throw_errno(const std::string& context = "") {
	std::string msg = context.empty() ? std::strerror(errno) : context + ": " + std::strerror(errno);
	throw std::runtime_error(msg);
}

// global var to indicate the cancel button has been pressed
inline std::atomic<bool> g_cancel{false};

// global var to signal progress to http
inline std::atomic<int> g_progress {};