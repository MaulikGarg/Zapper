#include "core.h"

#include "utility.h"

ByteFluxResult run_byteflux(std::string src, std::string dst, e_process process) {
	g_cancel = false;	 // reset global cancel
	g_progress = 0;	 // reset global progress
	g_byteflux_running = true;
	g_total_bytes = 0;
	g_bytes_completed = 0;

	ByteFluxResult result;

	// lambda to prepare the given string path
	auto pathify_string = [](std::string path) {
		auto start = path.find_first_not_of(" \t");
		auto end = path.find_last_not_of(" \t/");
		path = (start == std::string::npos) ? "" : path.substr(start, end - start + 1);
		return std::filesystem::path(path).lexically_normal();
	};

	try {
		IO_process mainprocess;

		// normalize and resolve paths
		mainprocess.m_source = pathify_string(src);
		resolve_source(mainprocess);

		mainprocess.m_destination = pathify_string(dst);

		// if the source is a regular file
		if (S_ISREG(mainprocess.m_source_info.st_mode)) {
			resolve_destination_file(mainprocess);
			g_total_bytes = mainprocess.m_source_info.st_size;
			if (process == e_process::copy)
				copy_file_engine(mainprocess);
			else if (process == e_process::move)
				move_file_engine(mainprocess);

			// if the source is a directory
		} else if (S_ISDIR(mainprocess.m_source_info.st_mode)) {
			// the main threadpool where file IO Processes will be pushed
			ThreadPool mainpool;
			resolve_destination_directory_root(mainprocess);

			if (process == e_process::copy) {
				// set total bytes to be transferred
				g_total_bytes = calculate_total_bytes(mainprocess.m_source);
				copy_directory_engine(mainprocess, mainpool);

			} else if (process == e_process::move) {
				// if the move is to a different device, calculates bytes to be moved
				if (!mainprocess.m_same_device)
					g_total_bytes = calculate_total_bytes(mainprocess.m_source);
				move_directory_engine(mainprocess, mainpool);
			}

			// shut down all threads that mainpool may have opened
			mainpool.shutdown();

			// collect errors from mainpool into a local vector
			std::vector<std::exception_ptr> errors = std::move(mainpool.get_errors());
			// iterate over the error vector
			for (auto err : errors) {
				if (err)
					try {
						// rethrow the current exception so the relative catch block can print it
						std::rethrow_exception(err);
					} catch (std::exception& e) {
						// push into file error vector of result
						result.file_errors.push_back(e.what());
					}
			}

			// if it was a move, safely destroy source since shutdown() is complete
			if (process == e_process::move) {
				if (result.file_errors.empty())
					std::filesystem::remove_all(mainprocess.m_source);
				else
					result.file_errors.push_back("Move incomplete, source not deleted due to errors.");
			}
		}
		// if the source is not a file or a directory
		else {
			throw_error("Unsupported format.");
		}
		result.m_success = true;  // byteflux has reached end

	} catch (std::exception& e) {
		result.m_success = false;
		result.fatal_error = e.what();
	}

	g_byteflux_running = false;
	return result;
}