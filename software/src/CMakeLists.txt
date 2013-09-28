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
		heston_types.hpp
		heston_both_acc.hpp
		heston_sl_acc.hpp
		heston_ml_acc.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
		heston_both_acc.cpp
		heston_sl_acc.cpp
		heston_ml_acc.cpp
)
add_library(heston_acc STATIC ${SOURCE} ${HEADERS})
target_link_libraries(heston_acc iodev)


#### lib:heston_sl_cpu ##############################################
set(HEADERS
		heston_types.hpp
		heston_sl_cpu.hpp
		../externals/ziggurat/gausszig_GSL.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
		heston_sl_cpu.cpp
)
add_library(heston_sl_cpu STATIC ${SOURCE} ${HEADERS})
target_link_libraries(heston_sl_cpu ${CMAKE_THREAD_LIBS_INIT})


#### lib:json_helper ##############################################
set(HEADERS
		json_helper.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
		json_helper.cpp
)
add_library(json_helper STATIC ${SOURCE} ${HEADERS})
target_link_libraries(json_helper jsoncpp)






#### exe:init_rng ######################################################
add_executable(init_rng init_rng.cpp)
target_link_libraries(init_rng iodev json_helper)
target_link_libraries(init_rng mt19937)

#### exe:run_heston ##############################################
add_executable(run_heston run_heston.cpp)
target_link_libraries(run_heston json_helper heston_sl_cpu)
if (WITH_ACC)
	target_link_libraries(run_heston heston_acc)
endif()

#### exe:read_fifo ######################################################
add_executable(read_fifo read_fifo.cpp)
target_link_libraries(read_fifo iodev)

#### exe:read_rng ######################################################
add_executable(read_rng read_rng.cpp)
target_link_libraries(read_rng iodev)
target_link_libraries(read_rng mt19937)

#### exe:read_heston ######################################################
add_executable(read_heston read_heston.cpp)
target_link_libraries(read_heston iodev)



### install targets ###

INSTALL(TARGETS init_rng run_heston read_fifo read_rng read_heston
		RUNTIME DESTINATION bin OPTIONAL)

