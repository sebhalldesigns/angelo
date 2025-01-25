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
elseif(UNIX)
    set(ANGELO_PLATFORM_SOURCE 
        src/app/app_unix.c
        src/win/win_unix.c

        src/misc/wayland/xdg-shell-protocol.c
        src/misc/wayland/kde-server-decoration.c
        src/misc/wayland/xdg-decoration.c
    )
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
elseif(UNIX)
    target_link_libraries(angelo X11 GL EGL wayland-client wayland-egl decor-0)
endif()

## ANGELO TEST

add_executable(angelo_test test/main.c)
target_include_directories(angelo_test PRIVATE src)
target_link_libraries(angelo_test angelo)
