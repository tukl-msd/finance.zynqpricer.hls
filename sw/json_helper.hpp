//
// Copyright (C) 2013 University of Kaiserslautern
// Microelectronic Systems Design Research Group
//
// Christian Brugger (brugger@eit.uni-kl.de)
// 29. August 2013
//

#ifndef __JSON_HELPER__
#define __JSON_HELPER__

#include "json/json.h"

#include <fstream>
#include <sstream>


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
	unsigned res;
	std::stringstream ss;
	ss << std::hex << val.asString();
	ss >> res;
	return res;
}


#endif
