cmake_minimum_required(VERSION 3.10)
project(io-multiplexing-demo C)

set(CMAKE_C_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/build/)

add_executable(echo_server_fork common.c echo_server_fork.c)
add_executable(echo_server_poll common.c echo_server_poll.c)
add_executable(echo_server_select common.c echo_server_select.c)

if(CMAKE_SYSTEM_NAME MATCHES "BSD" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
    add_executable(echo_server_kqueue common.c echo_server_kqueue.c)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_executable(echo_server_epoll common.c echo_server_epoll.c)
endif()

# https://cmake.org/cmake/help/latest/module/FindPkgConfig.html
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_search_module(LIBEVENT libevent)
    if(LIBEVENT_FOUND)
        include_directories(${LIBEVENT_INCLUDE_DIRS})
        link_directories(${LIBEVENT_LIBRARY_DIRS})
        add_executable(echo_server_libevent common.c echo_server_libevent.c)
        target_link_libraries(echo_server_libevent ${LIBEVENT_LIBRARIES})
    endif()
endif()