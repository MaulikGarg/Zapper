#include "server.h"

#include <cstdlib>
#include <string>

#include "../../external/httplib.h"

bool work_in_webapp() {
	httplib::Server svr;

	// try to get a random port assigned
	int port = svr.bind_to_any_port("localhost");

	if (!port)
		return false;	// port assignment failed

	// listener for the webapp
	std::thread listener(&httplib::Server::listen_after_bind, &svr);
	listener.detach();  // run seperately from primary

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