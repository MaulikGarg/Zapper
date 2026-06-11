#pragma once

#include <sys/stat.h>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <string_view>

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
inline std::atomic<int> g_progress{};

// global var to indicate byteflux is running
inline std::atomic<bool> g_byteflux_running{false};

// returns time since EPOCH in ms
inline int64_t get_time() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
				  std::chrono::steady_clock::now().time_since_epoch())
		 .count();
}