cmake_minimum_required(VERSION 3.0.0)
project(captain)

find_package(rpclib REQUIRED)

include_directories(${RPCLIB_INCLUDE_DIR})
AUX_SOURCE_DIRECTORY(. DIR_SRCS)
add_executable(captain ${DIR_SRCS})
target_link_libraries(captain ${RPCLIB_LIBS})
set_target_properties(
        captain
        PROPERTIES
        CXX_STANDARD 14
        COMPILE_FLAGS "${CMAKE_CXX_FLAGS} ${RPCLIB_EXTRA_FLAGS}")
target_compile_definitions(captain PUBLIC ${RPCLIB_COMPILE_DEFINITIONS})
