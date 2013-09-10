#
# Copyright (C) 2013 University of Kaiserslautern
# Microelectronic Systems Design Research Group
# 
# Christian Brugger (brugger@eit.uni-kl.de)
# 30. August 2013
#

include_directories(.)
include_directories(../externals)

find_package (Threads)

#### lib:iodev #######################################################
set(HEADERS
		iodev.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
		iodev.cpp
)
add_library(iodev STATIC ${SOURCE} ${HEADERS})


#### lib:heston_sl_acc ##############################################
set(HEADERS
		heston_sl_acc.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
		heston_sl_acc.cpp
)
add_library(heston_sl_acc ${SOURCE} ${HEADERS})
target_link_libraries(heston_sl_acc iodev)


#### lib:heston_ml_acc ##############################################
set(HEADERS
		heston_ml_acc.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
		heston_ml_acc.cpp
)
add_library(heston_ml_acc ${SOURCE} ${HEADERS})
target_link_libraries(heston_ml_acc iodev)


#### lib:heston_sl_cpu ##############################################
set(HEADERS
		heston_sl_cpu.hpp
		../externals/ziggurat/gausszig_GSL.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
		heston_sl_cpu.cpp
)
add_library(heston_sl_cpu ${SOURCE} ${HEADERS})
target_link_libraries(heston_sl_cpu ${CMAKE_THREAD_LIBS_INIT})


#### lib:json_helper ##############################################
set(HEADERS
		json_helper.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
		json_helper.cpp
)
add_library(json_helper ${SOURCE} ${HEADERS})
target_link_libraries(json_helper jsoncpp)






#### exe:init_rng ######################################################
add_executable(init_rng init_rng.cpp)
target_link_libraries(init_rng iodev json_helper)
target_link_libraries(init_rng mt19937)

#### exe:runner ##############################################
add_executable(run_heston run_heston.cpp)
target_link_libraries(run_heston json_helper heston_sl_cpu)
if (UNIX)
	target_link_libraries(run_heston heston_sl_acc heston_ml_acc)
endif()

#### exe:read_fifo ######################################################
add_executable(read_fifo read_fifo.cpp)
target_link_libraries(read_fifo iodev)

#### exe:read_rng ######################################################
add_executable(read_rng read_rng.cpp)
target_link_libraries(read_rng iodev)
target_link_libraries(read_rng mt19937)




### install targets ###

INSTALL(TARGETS init_rng run_heston read_fifo read_rng
		RUNTIME DESTINATION bin OPTIONAL)

