cmake_minimum_required(VERSION 2.8)
project(Path_ORAM)

# set(CMAKE_BUILD_TYPE Debug)
# SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb") 
# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall") 
set(CMAKE_CXX_STANDARD 11)

add_library(Path_ORAM_src
    src/Block.cpp
    src/Socket.cpp
    src/Client.cpp
    src/Server.cpp
    src/Log.cpp
)

add_executable(Path_ORAM src/main.cpp)
target_link_libraries(Path_ORAM Path_ORAM_src -ltomcrypt)

