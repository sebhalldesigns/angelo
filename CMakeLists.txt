cmake_minimum_required(VERSION 3.0)

project(angelo)

## ANGELO LIBRARY

if(APPLE)
    set(ANGELO_PLATFORM_SOURCE 
        src/app/app_macos.m
        src/win/win_macos.m
    )
elseif(WIN32)
    set(ANGELO_PLATFORM_SOURCE 
        src/app/app_win32.c
        src/win/win_win32.c
    )

    # prevent need for __declspec(dllexport) and import on Windows
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_library(angelo SHARED 

    src/debug/debug.c
    src/util/util.c

    ${ANGELO_PLATFORM_SOURCE}
)

if(APPLE)
    target_link_libraries(angelo "-framework Cocoa -framework Metal -framework QuartzCore")
elseif(WIN32)
    target_link_libraries(angelo user32 gdi32 opengl32)
endif()

## ANGELO TEST

add_executable(angelo_test test/main.c)
target_include_directories(angelo_test PRIVATE src)
target_link_libraries(angelo_test angelo)
