cmake_minimum_required (VERSION 2.8)
project (Outrigger)

set(CMAKE_THREAD_PREFER_PTHREAD)
find_package(Threads REQUIRED)

include_directories(
	api
	io
	io/serial
	rigs/kenwood_hf
	iniparser/src
)

set(SOURCES
	api/api.c
	io/io.c
	io/serial/serial.c
	rigs/kenwood_hf/kenwood_hf.c
	rigs/kenwood_hf/ts-940s.c
	iniparser/src/dictionary.c
	iniparser/src/iniparser.c
	test.c
)

find_path(TERMIOS_PATH termios.h)

if(CMAKE_USE_WIN32_THREADS_INIT)
endif()

if(TERMIOS_PATH)
	list(APPEND SOURCES io/serial/io_termios.c)
endif()

add_executable(outrigger ${SOURCES})

target_link_libraries(outrigger ${CMAKE_THREAD_LIBS_INIT})

if(TERMIOS_PATH)
	target_compile_definitions(outrigger PRIVATE WITH_TERMIOS)
endif()
