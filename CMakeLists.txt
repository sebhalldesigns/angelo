cmake_minimum_required(VERSION 3.0)

project(angelo)

## ANGELO LIBRARY
add_library(angelo SHARED 
    src/app/app_macos.m
    src/win/win_macos.m
)

if(APPLE)
    target_link_libraries(angelo "-framework Cocoa")
endif()

## ANGELO TEST
add_executable(angelo_test test/main.c)
target_include_directories(angelo_test PRIVATE src)
target_link_libraries(angelo_test angelo)