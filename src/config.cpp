#include <unordered_map>
#include <fstream>
#include "config.hpp"
#include <iostream> 
#include <string>
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
}

