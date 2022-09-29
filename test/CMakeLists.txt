cmake_minimum_required(VERSION 3.23)
project(test_client)

set(CMAKE_CXX_STANDARD 14)

add_executable(test_client
        main.cpp
        lib/lnp.h lib/lnpthread.h
        lib/wraplib.cpp lib/wrappthread.cpp lib/wrapsock.cpp lib/wrapstdio.cpp lib/wrapunix.cpp
        lib/writen.cpp lib/readn.cpp
        lib/error.cpp tcp_connect.cpp)

target_link_libraries(test_client -lpthread)
