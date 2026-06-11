cmake_minimum_required(VERSION 3.16)

project(ByteFlux)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(byteflux
    src/engine/main.cpp
    src/engine/validator.cpp
    src/engine/fileengine.cpp
    src/engine/ioprocess.cpp
    src/engine/threadpool.cpp
    src/engine/core.cpp
    src/webapp/server.cpp
)