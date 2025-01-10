add_library(libcdb process.cpp)
add_library(cdb::libcdb ALIAS libcdb)

set_target_properties(
	libcdb PROPERTIES OUTPUT_NAME cdb
)

target_compile_features(
	libcdb PUBLIC cxx_std_17
)

target_include_directories(
	libcdb
	PRIVATE include
	PUBLIC ${CMAKE_SOURCE_DIR}/include
)
