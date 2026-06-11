#pragma once

#include <cstdint>
#include "../../external/httplib.h"
#include "../engine/core.h"
#include "../../external/json.hpp"

// how many seconds the shutdown loop waits for before checking if shutdown is true
constexpr int SHUTDOWN_SLEEP {5};

// how many mseconds the system expects between heartbeats
constexpr int64_t TIME_BW_HEARTBEAT {10'000};

// tries to run byteflux as webapp, returns false on failure
// function only returns true when work is done in webapp
bool work_in_webapp();

// sets up the http server on a random port
bool open_webapp(httplib::Server& svr, int& port, std::thread& serverthread);