cmake_minimum_required(VERSION 3.16)

project(ByteFlux)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# shared engine sources used by both targets
set(ENGINE_SOURCES
    src/engine/validator.cpp
    src/engine/fileengine.cpp
    src/engine/ioprocess.cpp
    src/engine/threadpool.cpp
    src/engine/core.cpp
)

# CLI binary
add_executable(byteflux
    src/engine/main.cpp
    ${ENGINE_SOURCES}
)

# webapp binary
add_executable(byteflux_web
    src/webapp/main.cpp
    ${ENGINE_SOURCES}
)
