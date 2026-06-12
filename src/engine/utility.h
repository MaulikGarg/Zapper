#pragma once

#include <sys/stat.h>

#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string_view>

constexpr int PROGRESS_UPDATE_INTERVAL = 50;	 // in ms

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

// byte count for progress
inline std::atomic<uint64_t> g_bytes_completed{0};
inline std::atomic<uint64_t> g_total_bytes{0};
inline std::atomic<uint64_t> g_speed_bps{0};			  // bytes per second
inline std::atomic<int64_t> g_speed_last_time{0};	  // timestamp last sec
inline std::atomic<uint64_t> g_speed_last_bytes{0};  // bytes last second

// add n bytes to bytesfinished
inline void add_bytes_completed(uint64_t bytes) {
	g_bytes_completed.fetch_add(bytes, std::memory_order_relaxed);
}

// prints progress
inline void print_progress() {
	if (g_total_bytes.load()) {
		int percentage = ((double)g_bytes_completed.load() / g_total_bytes.load()) * 100;
		g_progress.store(percentage);
		std::cout << "\rProgress: " << percentage << "% " << std::flush;
	}

	// calculate and update speed
	int64_t now = get_time();
	int64_t elapsed = now - g_speed_last_time.load();
	if (elapsed >= PROGRESS_UPDATE_INTERVAL) {
		uint64_t current = g_bytes_completed.load();
		uint64_t bytesdiff = current - g_speed_last_bytes.load();
		uint64_t instant = (bytesdiff * 1000) / elapsed;
		uint64_t prev = g_speed_bps.load();
		g_speed_bps.store(prev == 0 ? instant : (prev * 3 + instant) / 4);
		g_speed_last_time.store(now);
		g_speed_last_bytes.store(current);
	}
}