cmake_minimum_required(VERSION 3.15)
project(socket_programming)

set(CMAKE_C_STANDARD 11)

include_directories(client)
include_directories(server)

add_executable(client
        client/client.c
        client/client.h
        client/main.c
        tools/tools.c)

add_executable(server
        server/main.c
        server/server.c
        server/server.h
        tools/tools.c)
