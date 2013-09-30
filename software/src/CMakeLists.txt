#
# Copyright (C) 2013 University of Kaiserslautern
# Microelectronic Systems Design Research Group
# 
# Christian Brugger (brugger@eit.uni-kl.de)
# 30. August 2013
#

include_directories(.)
include_directories(../externals)

find_package(Threads REQUIRED)

if (WITH_MPI)
	find_package(MPI REQUIRED)
	include_directories(${MPI_INCLUDE_PATH})
endif()

#### lib:iodev #######################################################
set(HEADERS
	iodev.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
	iodev.cpp
)
add_library(iodev STATIC ${SOURCE} ${HEADERS})


#### lib:heston_common ##############################################
set(HEADERS
		heston_common.hpp
		heston_ml_both.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
		heston_common.cpp
		heston_ml_both.cpp
)
add_library(heston_common STATIC ${SOURCE} ${HEADERS})


#### lib:heston_acc ##############################################
set(HEADERS
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
target_link_libraries(heston_acc heston_common)


#### lib:heston_cpu ##############################################
set(HEADERS
		heston_both_cpu.hpp
		heston_sl_cpu.hpp
		heston_ml_cpu.hpp
		../externals/ziggurat/gausszig_GSL.hpp
)
source_group("Header Files" FILES ${HEADERS})
set(SOURCE
		heston_both_cpu.cpp
		heston_sl_cpu.cpp
		heston_ml_cpu.cpp
)
add_library(heston_cpu STATIC ${SOURCE} ${HEADERS})
target_link_libraries(heston_cpu ${CMAKE_THREAD_LIBS_INIT})
if (WITH_MPI)
	target_link_libraries(heston_cpu ${MPI_LIBRARIES})
endif()
target_link_libraries(heston_cpu heston_common)


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
target_link_libraries(json_helper heston_common)






#### exe:init_rng ######################################################
add_executable(init_rng init_rng.cpp)
target_link_libraries(init_rng iodev json_helper)
target_link_libraries(init_rng mt19937)

#### exe:run_heston ##############################################
add_executable(run_heston run_heston.cpp)
target_link_libraries(run_heston json_helper heston_cpu)
if (WITH_ACC)
	target_link_libraries(run_heston heston_acc)
endif()
if (WITH_MPI)
	target_link_libraries(run_heston ${MPI_LIBRARIES})
endif()
target_link_libraries(heston_acc heston_common)

#### exe:eval_heston ##############################################
add_executable(eval_heston eval_heston.cpp)
target_link_libraries(eval_heston json_helper heston_cpu)
if (WITH_MPI)
	target_link_libraries(run_heston ${MPI_LIBRARIES})
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

INSTALL(TARGETS init_rng run_heston eval_heston read_fifo read_rng read_heston
		RUNTIME DESTINATION bin OPTIONAL)

