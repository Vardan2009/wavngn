cmake_minimum_required(VERSION 3.10)
project(wavngn)

add_library(wavngn STATIC
    src/wavngn.c
)

add_executable(wavngn_test
    main.c
)

target_link_libraries(wavngn_test PRIVATE wavngn)


if(UNIX AND NOT APPLE)
    target_link_libraries(wavngn m)
endif()