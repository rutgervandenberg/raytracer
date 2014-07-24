#include <unordered_map>
#include <fstream>
#include <iostream> 
#include <string>
#include "config.hpp"
#include "cpuinfo.hpp"

// load configuration from file
Config::Config(std::string configname) {
	std::unordered_map<std::string, int*> allowed_variables = {
			{"MSAA", &MSAA},
			{"WINDOW_RES_X", &WINDOW_RES_X},
			{"WINDOW_RES_Y", &WINDOW_RES_Y},
			{"RAYTRACE_RES_X", &RAYTRACE_RES_X},
			{"RAYTRACE_RES_Y", &RAYTRACE_RES_Y},
			{"VISIBILITY_TRESHOLD", &VISIBILITY_TRESHOLD}
	};

	std::ifstream in(configname);
	std::string optionname;
	int* optionptr;
	int value;

	while (in >> optionname) {
		try {
			optionptr = allowed_variables.at(optionname);
		} catch (const std::out_of_range& oor) {
			std::cerr << "Illegal option \"" << optionname << "\" in"
					<< configname << ": " << oor.what() << '\n';
			continue;
		}
		in >> value;

		if (!in.good()) {
			std::cout << "Incorrect config file: " << configname
					<< " at option: " << optionname << '\n';
		} else {
			*optionptr = value;
		}
	}
	in.close();

	// check for cpu info
	checkCpuInfo(this);

	// print information
	printf("numcores = %d\nsse = %d\nsse2 = %d\nsse3 = %d\nsse4 = %d\navx = %d\navx512 = %d\n",
		numcores, sse, sse2, sse3, sse4, avx, avx512);
}

