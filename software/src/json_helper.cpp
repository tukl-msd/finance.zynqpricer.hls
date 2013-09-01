//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 29. August 2013
//

#include "json_helper.hpp"

#include "json/json.h"

#include <fstream>
#include <sstream>
#include <string>


Json::Value read_params(char *filename) {
	std::ifstream file(filename);
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(file, root)) {
		std::cerr << "Failed to parse parameter file" << std::endl
				<< reader.getFormattedErrorMessages();
		exit(-1);
	}
	return root;
}


unsigned asHex(Json::Value val) {
	return std::stoul(val.asString(), 0, 0);
}


