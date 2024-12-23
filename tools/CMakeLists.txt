add_executable(cdb cdb.cpp)
target_link_libraries(cdb PRIVATE cdb::libcdb PkgConfig::libedit)
