cmake_minimum_required (VERSION 2.8)
project (Outrigger)

set(CMAKE_THREAD_PREFER_PTHREAD)
find_package(Threads REQUIRED)

include_directories(
	api
	iniparser/src
	io
	io/serial
	os
	rigs/kenwood_hf
)

set(SOURCES
	api/api.c
	io/io.c
	io/serial/serial.c
	rigs/kenwood_hf/kenwood_hf.c
	rigs/kenwood_hf/ts-940s.c
	iniparser/src/dictionary.c
	iniparser/src/iniparser.c
)

find_path(TERMIOS_PATH termios.h)
find_path(UNISTD_PATH unistd.h)

if(CMAKE_USE_WIN32_THREADS_INIT)
endif()

if(TERMIOS_PATH)
	list(APPEND SOURCES io/serial/io_termios.c)
endif()

if(UNISTD_PATH)
	check_symbol_exists(_POSIX_TIMERS ${UNISTD_PATH}/unistd.h HAS_POSIX_TIMERS)
	if(HAS_POSIX_TIMERS)
		list(APPEND SOURCES os/datetime_posix.c)
	endif()
	list(APPEND CMAKE_REQUIRED_LIBRARIES c)
	check_symbol_exists(fork ${UNISTD_PATH}/unistd.h HAS_FORK)
endif()

add_library(outrigger ${SOURCES})
add_executable(test test.c)
target_link_libraries(test outrigger)
add_executable(or-rigctld or-rigctld.c)
target_link_libraries(or-rigctld outrigger)

target_link_libraries(outrigger ${CMAKE_THREAD_LIBS_INIT})

if(TERMIOS_PATH)
	target_compile_definitions(outrigger PRIVATE WITH_TERMIOS)
endif()

if(HAS_FORK)
	target_compile_definitions(or-rigctld PRIVATE WITH_FORK)
endif()
