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


#### lib:heston_sl_cpu ##############################################
set(HEADERS
		heston_sl_cpu.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
		heston_sl_cpu.cpp
)
add_library(heston_sl_cpu ${SOURCE} ${HEADERS})
target_link_libraries(heston_sl_cpu)
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


#### lib:runner ##############################################
set(HEADERS
		runner.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
		runner.cpp
)
add_library(runner ${SOURCE} ${HEADERS})
target_link_libraries(runner json_helper heston_sl_cpu)
if (UNIX)
	target_link_libraries(runner heston_sl_acc)
endif()






#### exe:init_rng ######################################################
add_executable(init_rng init_rng.cpp)
target_link_libraries(init_rng iodev json_helper)
target_link_libraries(init_rng mt19937)

#### exe:run_cpu ######################################################
add_executable(run_cpu run_cpu.cpp)
target_link_libraries(run_cpu heston_sl_cpu runner)

#### exe:run_acc ######################################################
add_executable(run_acc run_acc.cpp)
target_link_libraries(run_acc heston_sl_acc runner)

#### exe:run_both ######################################################
add_executable(run_both run_both.cpp)
target_link_libraries(run_both heston_sl_acc heston_sl_cpu runner)

#### exe:read_fifo ######################################################
add_executable(read_fifo read_fifo.cpp)
target_link_libraries(read_fifo iodev)

#### exe:read_rng ######################################################
add_executable(read_rng read_rng.cpp)
target_link_libraries(read_rng iodev)
target_link_libraries(read_rng mt19937)




### install targets ###

INSTALL(TARGETS init_rng run_cpu run_acc run_both read_fifo read_rng
		RUNTIME DESTINATION bin OPTIONAL)

