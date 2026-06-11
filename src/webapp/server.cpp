#include "server.h"
using json = nlohmann::json;

bool open_webapp(httplib::Server& svr, int& port, std::thread& serverthread) {
	// try to get a random port assigned
	port = svr.bind_to_any_port("localhost");

	if (!port)
		return false;	// port assignment failed

	// listener for the webapp
	serverthread = std::thread(&httplib::Server::listen_after_bind, &svr);
	serverthread.detach();	// run seperately from primary

	// open app for webapp local url
	std::string url = "http://localhost:" + std::to_string(port);

	// check if xdg-open even exists
	if (system("which xdg-open > /dev/null 2>&1") != 0) {
		svr.stop();
		return false;
	}

	system(("xdg-open " + url).c_str());
	return true;
}

bool work_in_webapp() {
	httplib::Server svr;
	int port{};
	std::thread listener;
	std::atomic<bool> shutdown{false};
	std::atomic<int64_t> last_heartbeat{get_time()};

	auto heartbeat = [&](const httplib::Request&, httplib::Response&) {
		last_heartbeat = get_time();
	};

	auto quit = [&](const httplib::Request&, httplib::Response&) {
		shutdown.store(true);
	};

	auto progress = [](const httplib::Request&, httplib::Response& res) {
		// send progress percentage
		res.set_content(std::to_string(g_progress.load()), "text/plain");
	};

	auto transfer = [](const httplib::Request& req, httplib::Response& res) {
		std::string source = req.get_param_value("src");
		std::string destination = req.get_param_value("dst");
		e_process mode = req.get_param_value("mode") == "c" ? e_process::copy : e_process::move;
		ByteFluxResult result = run_byteflux(source, destination, mode);

		json json_result;
		json_result["success"] = result.m_success;
		json_result["fatal_error"] = result.fatal_error;
		json_result["file_errors"] = result.file_errors;  // nlohmann handles vectors natively
		res.set_content(json_result.dump(), "application/json");
	};

	svr.Get("/heartbeat", heartbeat);
	svr.Get("/quit", quit);
	svr.Get("/progress", progress);
	svr.Post("/transfer", transfer);

	if (!open_webapp(svr, port, listener))
		return false;

	while (!shutdown.load()) {
		std::this_thread::sleep_for(std::chrono::seconds(SHUTDOWN_SLEEP));
		if (get_time() - last_heartbeat.load() > TIME_BW_HEARTBEAT)
			break;
	}

	g_cancel = true;	// treat any quit as CANCEL

	// wait for byteflux to stop running
	while (g_byteflux_running.load())
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// shutdown server now that everything is done
	svr.stop();
	return true;
}