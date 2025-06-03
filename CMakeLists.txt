cmake_minimum_required(VERSION 3.5)
project(wavngn)

add_executable(${PROJECT_NAME} main.c src/wavngn.c)

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} m)
endif()