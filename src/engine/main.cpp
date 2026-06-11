#include <iostream>

#include "core.h"
#include "../webapp/server.h"

enum e_whichpath {
	source,
	destination
};

void get_path(std::string& path, e_whichpath which);

int main(int argc, char* argv[]) {

	// if work done in webapp, exit 
	if(work_in_webapp()) return 0;
	// if false, do CLI
	std::string src{}, dst{}, flag{};

	std::cout << "Byteflux v0.6\n";

	if (argc == 4) {
		flag = argv[1];
		src = argv[2];
		dst = argv[3];
	} else {
		get_path(src, source);
		get_path(dst, destination);
		std::cout << "Enter the desired process(-c for copy, -m for move): ";
		std::cin >> flag;
	}

	e_process process;
	if (flag == "-c")
		process = e_process::copy;
	else if (flag == "-m")
		process = e_process::move;
	else {
		std::cerr << "Invalid flag: " + flag << '\n';
		return 1;
	}

	ByteFluxResult result = run_byteflux(src, dst, process);

	if (!result.m_success) {
		std::cerr << result.fatal_error << '\n';
		return 1;
	}

	// print file errors
	for (const auto& err : result.file_errors)
		std::cerr << err << '\n';

	std::cout << "Exiting...\n";
	return 0;
}

void get_path(std::string& path, e_whichpath which) {
	switch (which) {
		case source:
			std::cout << "Enter the source file path: ";
			break;
		case destination:
			std::cout << "Enter the destination file path: ";
			break;
	}
	getline(std::cin, path);
}
