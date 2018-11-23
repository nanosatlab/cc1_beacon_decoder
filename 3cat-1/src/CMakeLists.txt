file (GLOB_RECURSE beacon_decoder *.c)
add_executable(beacon_decoder ${beacon_decoder})
target_include_directories(beacon_decoder PRIVATE ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_link_libraries(beacon_decoder PRIVATE ${LIBMYSQLCLIENT_LIBRARIES})
