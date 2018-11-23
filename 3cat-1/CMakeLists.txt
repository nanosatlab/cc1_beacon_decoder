cmake_minimum_required(VERSION 2.6)

project(BEACON3CAT-1 C)

include_directories(/usr/include)
include(FindPkgConfig)
pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)

foreach(FLAG ${LIBMYSQLCLIENT_CFLAGS_OTHER})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
endforeach()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/)

if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}")
  endif ()
else ()
  set (CMAKE_CXX_STANDARD 11)
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

link_directories(/usr/lib/)
link_directories(/usr/local/lib/)
link_directories(${LIBMYSQLCLIENT_LIBRARY_DIRS})

add_subdirectory(src)
