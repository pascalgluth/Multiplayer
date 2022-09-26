cmake_minimum_required(VERSION 3.21)
project(GameServer)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../sockpp/include)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE sockpp)
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE sockpp-static)
endif()