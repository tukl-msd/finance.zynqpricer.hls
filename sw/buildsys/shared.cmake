#
# Copyright (C) 2013 University of Kaiserslautern
# Microelectronic Systems Design Research Group
# 
# Christian Brugger (brugger@eit.uni-kl.de)
# 30. August 2013
#

cmake_minimum_required(VERSION 2.8.3)

# add path to custom FindXXX.cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
		"${CMAKE_CURRENT_LIST_DIR}/cmake/Modules/")

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(SEND_ERROR "In-source builds are not allowed.\n"
			"First remove the files already created:\n"
			"Please run the following commands:\n"
			"\trm -rf CMakeCache.txt CMakeFiles\n"
			"\tmkdir build\n"
			"\tcd build\n"
			"\tcmake ..")
endif()


