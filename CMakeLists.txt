cmake_minimum_required(VERSION 3.0)

project(angelo)

## ANGELO LIBRARY
add_library(angelo SHARED 

    src/debug/debug.c
    src/util/util.c

    src/app/app_macos.m
    src/win/win_macos.m
)

if(APPLE)
    target_link_libraries(angelo "-framework Cocoa -framework Metal -framework QuartzCore")
endif()

## ANGELO TEST
add_executable(angelo_test test/main.c)
target_include_directories(angelo_test PRIVATE src)
target_link_libraries(angelo_test angelo)