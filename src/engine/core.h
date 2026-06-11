#pragma once

#include "fileengine.h"

enum class e_process {
		copy,
		move
	};

struct ByteFluxResult{
    bool m_success {false};
    // fatal crash error
    std::string fatal_error {}; 
    // list of errors in dir case
    std::vector<std::string> file_errors {}; 
};

ByteFluxResult run_byteflux(std::string src, std::string dst, e_process prc);