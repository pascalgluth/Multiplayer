cmake_minimum_required(VERSION 3.21)
project(GameClient)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME}
        main.cpp
        Player.cpp
        Timer.cpp
        Chat.cpp Chat.h
        Global.h)

target_include_directories(${PROJECT_NAME} PRIVATE ../sockpp/include)

if(APPLE)
    include_directories(/usr/local/include)

    find_package(SFML 2.5 COMPONENTS system window graphics network audio REQUIRED)
    include_directories(${SFML_INCLUDE_DIRS})

    target_link_libraries(${PROJECT_NAME} sockpp sfml-system sfml-window sfml-graphics sfml-audio sfml-network)
endif()

if(WIN32)

    #include_directories(headers C:/Libs/SFML/include)
    #set(SFML_ROOT C:/Libs/SFML)

    #find_package(SFML REQUIRED audio network graphics window system)
    #if(SFML_FOUND)
    #    include_directories(${SFML_INCLUDE_DIR})
    #    #target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
    #    target_link_libraries(2DRPGGame sfml-system sfml-window sfml-graphics sfml-audio sfml-network)

    #endif()


    set(SFML_ROOT C:/Libs/SFML-VS)
    set(CMAKE_MODULE_PATH ../cmake_modules)
    find_package(SFML REQUIRED audio network graphics window system)
    if (SFML_FOUND)
        include_directories(${SFML_INCLUDE_DIR})
        target_link_libraries(${PROJECT_NAME} sockpp-static ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
    endif()

    # Copy dlls to build

    file(GLOB BINARY_DEP_DLLS "${SFML_INCLUDE_DIR}/../bin/*.dll")
    file(COPY ${BINARY_DEP_DLLS} DESTINATION ${CMAKE_BINARY_DIR})
    #file(GLOB MINGW_DEP_DLLS "C:/mingw64/bin/*.dll")
    #file(COPY ${MINGW_DEP_DLLS} DESTINATION ${CMAKE_BINARY_DIR})

endif()