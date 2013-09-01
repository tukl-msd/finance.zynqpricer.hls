#
# Copyright (C) 2013 University of Kaiserslautern
# Microelectronic Systems Design Research Group
# 
# Christian Brugger (brugger@eit.uni-kl.de)
# 30. August 2013
#

include_directories(.)

add_library(jsoncpp STATIC json/jsoncpp.cpp json/json.h json/json-forwards.h)

add_library(mt19937 STATIC mt19937/mt19937ar.cpp mt19937/mt19937ar.h)

add_library(ziggurat STATIC ziggurat/gausszig_GSL.cpp  
		ziggurat/gausszig_GSL.hpp)