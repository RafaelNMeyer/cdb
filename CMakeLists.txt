cmake_minimum_required(VERSION 3.19)

project("cdb" CXX)

include(FindPkgConfig)

pkg_check_modules(libedit REQUIRED IMPORTED_TARGET libedit)

include(CTest)

add_subdirectory("src")
add_subdirectory("tools")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(BUILD_TESTING)
	find_package(Catch2 CONFIG REQUIRED)
	add_subdirectory("test")
endif()
