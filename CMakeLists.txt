cmake_minimum_required(VERSION 2.6)
project(wavngn)

add_executable(${PROJECT_NAME} main.c src/wavngn.c)
target_link_libraries(${PROJECT_NAME} m)
