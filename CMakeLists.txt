cmake_minimum_required(VERSION 3.0.0)
project(net-client VERSION 0.1.0)

set(TARGET_NAME net-server)

include(CTest)
enable_testing()

add_executable(${TARGET_NAME} main.cpp Server.cpp CircularLineBuffer.cpp socket.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

if(WIN32)
    target_link_libraries(${TARGET_NAME} Ws2_32)
else()
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET_NAME} Threads::Threads)
endif()