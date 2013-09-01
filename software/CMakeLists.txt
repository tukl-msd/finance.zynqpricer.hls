#
# Copyright (C) 2013 University of Kaiserslautern
# Microelectronic Systems Design Research Group
# 
# Christian Brugger (brugger@eit.uni-kl.de)
# 30. August 2013
#

cmake_minimum_required(VERSION 2.8)
project(finance.zynqpricer.hls)

include(buildsys/shared.cmake)

#SET(CMAKE_VERBOSE_MAKEFILE ON)

# only install targets that have been build
SET(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)


# g++ compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -ffast-math")
	# set flags for Zynq
	if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "armv7l")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon")
	endif()
endif()

# msvc compiler flags
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /arch:AVX /fp:fast /GL")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
	set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /LTCG")
endif()

# enable_testing()
add_subdirectory(src)
add_subdirectory(externals)

