cmake_minimum_required(VERSION 3.21)
project(MPTestGame)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(sockpp)
add_subdirectory(GameServer)
add_subdirectory(GameClient)