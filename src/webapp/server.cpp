#include "server.h"
using json = nlohmann::json;

// webpage files
static const unsigned char index_html[] = {
#embed "../webapp/index.html"
	 , '\0'};
static const unsigned char styles_css[] = {
#embed "../webapp/styles.css"
	 , '\0'};
static const unsigned char styles_js[] = {
#embed "../webapp/styles.js"
	 , '\0'};
static const unsigned char byteflux_js[] = {
#embed "../webapp/byteflux.js"
	 , '\0'};

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

void register_static_routes(httplib::Server& svr) {
	svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
		res.set_content((const char*)index_html, "text/html");
	});
	svr.Get("/styles.css", [](const httplib::Request&, httplib::Response& res) {
		res.set_content((const char*)styles_css, "text/css");
	});
	svr.Get("/styles.js", [](const httplib::Request&, httplib::Response& res) {
		res.set_content((const char*)styles_js, "text/javascript");
	});
	svr.Get("/byteflux.js", [](const httplib::Request&, httplib::Response& res) {
		res.set_content((const char*)byteflux_js, "text/javascript");
	});
}

void register_api_routes(httplib::Server& svr, std::atomic<bool>& shutdown, std::atomic<int64_t>& last_heartbeat) {
	// calls byteflux from webpage data and sends back result
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

	// updates heartbeat time whenever the webpage beats
	auto heartbeat = [&](const httplib::Request&, httplib::Response&) {
		last_heartbeat = get_time();
	};

	// sets shutdown var to true which initiates shutdown sequence
	auto quit = [&](const httplib::Request&, httplib::Response&) {
		shutdown.store(true);
	};

	// sends progress report to webpage
	auto progress = [](const httplib::Request&, httplib::Response& res) {
		// send progress percentage
		res.set_content(std::to_string(g_progress.load()), "text/plain");
	};

	svr.Get("/heartbeat", heartbeat);
	svr.Get("/quit", quit);
	svr.Get("/progress", progress);
	svr.Post("/transfer", transfer);
}

bool work_in_webapp() {
	httplib::Server svr;
	int port{};
	std::thread listener;
	std::atomic<bool> shutdown{false};
	std::atomic<int64_t> last_heartbeat{get_time()};

	register_static_routes(svr);
	register_api_routes(svr, shutdown, last_heartbeat);

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