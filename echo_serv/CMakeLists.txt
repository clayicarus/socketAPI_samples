cmake_minimum_required(VERSION 3.23)
project(echo_serv)

set(CMAKE_CXX_STANDARD 14)

add_executable(echo_serv
        main_poll.cpp
        lib/lnp.h lib/lnpthread.h
        lib/wraplib.cpp lib/wrappthread.cpp lib/wrapsock.cpp lib/wrapstdio.cpp lib/wrapunix.cpp
        lib/writen.cpp
        lib/error.cpp)

target_link_libraries(echo_serv -lpthread)
