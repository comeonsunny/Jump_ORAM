cmake_minimum_required(VERSION 2.8)
project(Jump_ORAM)

# set(CMAKE_BUILD_TYPE Debug)
# SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb") 
# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall") 
set(CMAKE_CXX_STANDARD 11)
# make the data/JUMPORAM and log/JUMPORAM directory to store the data and log files
# set(JUMPORAM_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data/JUMPORAM)
# set(JUMPORAM_LOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/log/JUMPORAM)
set(JUMPORAM_DATA_DIR ${CMAKE_BINARY_DIR}/data/JUMPORAM)
set(JUMPORAM_LOG_DIR ${CMAKE_BINARY_DIR}/log/JUMPORAM)
if(NOT EXISTS ${JUMPORAM_DATA_DIR})
    file(MAKE_DIRECTORY ${JUMPORAM_DATA_DIR})
endif()
if(NOT EXISTS ${JUMPORAM_LOG_DIR})
    file( MAKE_DIRECTORY ${JUMPORAM_LOG_DIR})
endif()
# set(JUMPORAM_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data/JUMPORAM)
# if(NOT EXISTS ${JUMPORAM_DATA_DIR})
#     message("Creating ${JUMPORAM_DATA_DIR}")
#     mkdir(${JUMPORAM_DATA_DIR})

add_library(Jump_ORAM_src
    src/client.cc
    src/jump_oram.cc
    src/server.cc
    src/utils.cc
)

add_executable(Jump_ORAM src/main.cc)
target_link_libraries(Jump_ORAM Jump_ORAM_src -ltomcrypt)

