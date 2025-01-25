/***************************************************************
**
** Angelo Library Source File
**
** File         :  source_template.c
** Module       :  misc
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-08 (YYYY-MM-DD)
** License      :  MIT
** Description  :  A source file template.
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "win.h"
#include "../debug/debug.h"
#include "../util/util.h"


#define WIN32_LEAN_AND_MEAN

#ifndef UNICODE
#define UNICODE
#endif 

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <windows.h>
#include <windowsx.h>
#include <gl/gl.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

const LPWSTR WINDOW_CLASS_NAME = L"AngeloWindowClass";

// n.b this code sourced from https://github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c
typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
        const int *attribList);
wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;


// See https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt for all values
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList,
        const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

// See https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt for all values
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

bool hasRegisteredWindowClass = false;
HINSTANCE hInstance = NULL;

float mouseX = 0.0f;
float mouseY = 0.0f;

int frameTimeUs = 0;

HDC gldc;

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static void ensure_window_class();
static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* note this uses MALLOC */
static LPWSTR create_wide_string(const char* str);

static HGLRC
init_opengl(HDC real_dc);

static void
init_opengl_extensions(void);

static void
fatal_error(char *msg)
{
    MessageBoxA(NULL, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
    exit(EXIT_FAILURE);
}

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

WindowHandle_opt create_window(int width, int height, const char* title)
{
    
    ensure_window_class();

    LPWSTR wtitle = create_wide_string(title);

    HWND hwnd = CreateWindowW(
        WINDOW_CLASS_NAME,
        wtitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, hInstance, NULL
    );

    free(wtitle);

    if (hwnd == NULL)
    {
        log_error("Failed to create window");
        return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
    }

    log_info("Window created");

    gldc = GetDC(hwnd);
    HGLRC glrc = init_opengl(gldc);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = true };
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

static void ensure_window_class()
{
    if (hasRegisteredWindowClass)
    {
        return;
    }

    hInstance = GetModuleHandle(NULL);
    
    WNDCLASSW wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = window_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClassW(&wc))
    {
        log_error("Failed to register window class");
        return;
    }

    hasRegisteredWindowClass = true;

    log_info("Window class registered");
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);

            log_info("Window closed");
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_MOUSEMOVE:
        {
            mouseX = GET_X_LPARAM(lParam);
            mouseY = GET_Y_LPARAM(lParam);

            log_info("Mouse moved to (%f, %f)", mouseX, mouseY);
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);

            glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Do OpenGL rendering here

            SwapBuffers(gldc);

            log_info("Window painted");
            break;
        }
        default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    return 0;
}

static LPWSTR create_wide_string(const char* str)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    LPWSTR wstr = malloc(size * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, size);
    return wstr;
}

static void
init_opengl_extensions(void)
{
    // Before we can load extensions, we need a dummy OpenGL context, created using a dummy window.
    // We use a dummy window because you can only set the pixel format for a window once. For the
    // real window, we want to use wglChoosePixelFormatARB (so we can potentially specify options
    // that aren't available in PIXELFORMATDESCRIPTOR), but we can't load and use that before we
    // have a context.
    WNDCLASSA window_class = {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = DefWindowProcA,
        .hInstance = GetModuleHandle(0),
        .lpszClassName = "Dummy_WGL_djuasiodwa",
    };

    if (!RegisterClassA(&window_class)) {
        fatal_error("Failed to register dummy OpenGL window.");
    }

    HWND dummy_window = CreateWindowExA(
        0,
        window_class.lpszClassName,
        "Dummy OpenGL Window",
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        window_class.hInstance,
        0);

    if (!dummy_window) {
        fatal_error("Failed to create dummy OpenGL window.");
    }

    HDC dummy_dc = GetDC(dummy_window);

    PIXELFORMATDESCRIPTOR pfd = {
        .nSize = sizeof(pfd),
        .nVersion = 1,
        .iPixelType = PFD_TYPE_RGBA,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .cColorBits = 32,
        .cAlphaBits = 8,
        .iLayerType = PFD_MAIN_PLANE,
        .cDepthBits = 24,
        .cStencilBits = 8,
    };

    int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
    if (!pixel_format) {
        fatal_error("Failed to find a suitable pixel format.");
    }
    if (!SetPixelFormat(dummy_dc, pixel_format, &pfd)) {
        fatal_error("Failed to set the pixel format.");
    }

    HGLRC dummy_context = wglCreateContext(dummy_dc);
    if (!dummy_context) {
        fatal_error("Failed to create a dummy OpenGL rendering context.");
    }

    if (!wglMakeCurrent(dummy_dc, dummy_context)) {
        fatal_error("Failed to activate dummy OpenGL rendering context.");
    }

    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
        "wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
        "wglChoosePixelFormatARB");

    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_context);
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);
}

static HGLRC
init_opengl(HDC real_dc)
{
    init_opengl_extensions();

    // Now we can choose a pixel format the modern way, using wglChoosePixelFormatARB.
    int pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        0
    };

    int pixel_format;
    UINT num_formats;
    wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
    if (!num_formats) {
        fatal_error("Failed to set the OpenGL 3.3 pixel format.");
    }

    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(real_dc, pixel_format, sizeof(pfd), &pfd);
    if (!SetPixelFormat(real_dc, pixel_format, &pfd)) {
        fatal_error("Failed to set the OpenGL 3.3 pixel format.");
    }

    // Specify that we want to create an OpenGL 3.3 core profile context
    int gl33_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    HGLRC gl33_context = wglCreateContextAttribsARB(real_dc, 0, gl33_attribs);
    if (!gl33_context) {
        fatal_error("Failed to create OpenGL 3.3 context.");
    }

    if (!wglMakeCurrent(real_dc, gl33_context)) {
        fatal_error("Failed to activate OpenGL 3.3 rendering context.");
    }

    return gl33_context;
}