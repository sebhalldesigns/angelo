/***************************************************************
**
** Angelo Library Source File
**
** File         :  win_unix.c
** Module       :  win
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-08 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Unix window implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "win.h"
#include "win_unix.h"
#include "../app/app_unix.h"

#include "../debug/debug.h"
#include "../util/util.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

/* GLES config attributes */
const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_NONE
};

/* GLES context attributes */
const EGLint contextAttribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE,
    EGL_NONE
};

/* OpenGL 3.3 context attributes */
static int context_attribs[] = {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 3,
    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    None
};

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

WindowHandle_opt create_window(AppHandle app, int width, int height, const char* title)
{
    
    UnixApp* unixApp = (UnixApp*)app;
    if (unixApp == NULL || unixApp->appType == UNIX_APP_UNDEFINED)
    {
        log_error("Invalid app handle");
        return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
    }

    if (unixApp->appType == UNIX_APP_WAYLAND)
    {
        /* create a Wayland window */

        log_info("Creating Wayland window");


        UnixWindow* unixWindow = malloc(sizeof(UnixWindow));
        unixWindow->title = title;
        unixWindow->appType = UNIX_APP_WAYLAND;
        unixWindow->data.waylandData.wl_display = unixApp->data.waylandData.display;
        unixWindow->data.waylandData.wl_surface = wl_compositor_create_surface(unixApp->data.waylandData.compositor);

        unixWindow->data.waylandData.xd_surface = xdg_wm_base_get_xdg_surface(
            unixApp->data.waylandData.xdWmBase, 
            unixWindow->data.waylandData.wl_surface
        );

        xdg_surface_add_listener(unixWindow->data.waylandData.xd_surface, &xdg_surface_listener, unixWindow);
        
        unixWindow->data.waylandData.xd_toplevel = xdg_surface_get_toplevel(unixWindow->data.waylandData.xd_surface);
        xdg_toplevel_set_title(unixWindow->data.waylandData.xd_toplevel, title);

        if (unixApp->data.waylandData.kdeDecorations != NULL)
        {
            org_kde_kwin_server_decoration_manager_create(unixApp->data.waylandData.kdeDecorations, unixWindow->data.waylandData.wl_surface);
        }
        
        if (unixApp->data.waylandData.zxdgDecorations != NULL) {

            struct zxdg_toplevel_decoration_v1* decoration =
                zxdg_decoration_manager_v1_get_toplevel_decoration(
                    unixApp->data.waylandData.zxdgDecorations,
                    unixWindow->data.waylandData.xd_toplevel
                );

            zxdg_toplevel_decoration_v1_set_mode(
                decoration,
                ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE
            );
        }
        
        xdg_toplevel_add_listener(unixWindow->data.waylandData.xd_toplevel, &toplevelListener, unixApp);

        wl_surface_commit(unixWindow->data.waylandData.wl_surface);
        wl_display_roundtrip(unixWindow->data.waylandData.wl_display);

        /* init EGL */
        EGLDisplay eglDisplay = eglGetDisplay((EGLNativeDisplayType)unixApp->data.waylandData.display);
        if (eglDisplay == EGL_NO_DISPLAY) {
            log_error("Failed to get EGL display");
            return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        if (!eglInitialize(eglDisplay, NULL, NULL)) {
            log_error("Failed to initialize EGL");
            return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        EGLConfig eglConfig;
        EGLint numConfigs;
        if (!eglChooseConfig(eglDisplay, configAttribs, &eglConfig, 1, &numConfigs) || numConfigs == 0) {
            log_error("Failed to choose EGL config");
            return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        struct wl_egl_window* eglWindow = wl_egl_window_create(
            unixWindow->data.waylandData.wl_surface,
            width, height
        );
        if (!eglWindow) {
            log_error("Failed to create Wayland EGL window");
            return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        EGLSurface eglSurface = eglCreateWindowSurface(
            eglDisplay, eglConfig, (EGLNativeWindowType)eglWindow, NULL
        );
        if (eglSurface == EGL_NO_SURFACE) {
            log_error("Failed to create EGL surface");
            return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        EGLContext eglContext = eglCreateContext(
            eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs
        );
        if (eglContext == EGL_NO_CONTEXT) {
            log_error("Failed to create EGL context");
            return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
            log_error("Failed to make EGL context current");
            return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        log_info("Initialized OpenGL %s", glGetString(GL_VERSION));

        unixWindow->data.waylandData.eglDisplay = eglDisplay;
        unixWindow->data.waylandData.eglSurface = eglSurface;
        unixWindow->data.waylandData.eglContext = eglContext;
        unixWindow->data.waylandData.eglWindow = eglWindow;

        struct libdecor_frame *frame = libdecor_decorate(unixApp->data.waylandData.decor_context, unixWindow->data.waylandData.wl_surface, &frame_interface, unixApp);
        if (!frame) {
            log_error("Invalid app handle");
            return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        unixWindow->data.waylandData.frame = frame;

        
        // Set initial window state and size
        libdecor_frame_set_app_id(frame, "libdecor-example");
        libdecor_frame_set_title(frame, "My Window");
        libdecor_frame_set_min_content_size(frame, width, height);

        libdecor_frame_map(frame);

        unixApp->windowHandle = (WindowHandle)unixWindow; 

        wl_surface_commit(unixWindow->data.waylandData.wl_surface);
        wl_display_roundtrip(unixWindow->data.waylandData.wl_display);

        log_info("Created Wayland window");


        return (WindowHandle_opt) { .value = (intptr_t)unixWindow, .is_some = true };
    }
    else
    {
        /* Create an Xorg window */

        Window window = XCreateWindow(
            unixApp->data.xorgData.display, unixApp->data.xorgData.root,    /* parent */ 
            0, 0,                               /* position */ 
            width, height,                      /* size */ 
            0,                                  /* border size */  
            unixApp->data.xorgData.visualInfo->depth,         /* depth */ 
            InputOutput,                        /* class */ 
            unixApp->data.xorgData.visualInfo->visual,        /* visual */ 
            CWEventMask | CWColormap,           
            &unixApp->data.xorgData.windowAttributes
        );

        Atom deleteAtom = XInternAtom(unixApp->data.xorgData.display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(unixApp->data.xorgData.display, window, &deleteAtom, 1);
        
        XStoreName(unixApp->data.xorgData.display, window, title);
        XMapWindow(unixApp->data.xorgData.display, window);

        /* Create an OpenGL 3.3 context */

        PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = NULL;
        glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");

        if (!glXCreateContextAttribsARB) {
            log_error("glXCreateContextAttribsARB not found");
            return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        GLXContext context = glXCreateContextAttribsARB(unixApp->data.xorgData.display, unixApp->data.xorgData.bestFbc, NULL, True, context_attribs);
        if (!context) {
            log_error("Failed to create OpenGL context");
            return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        glXMakeCurrent(unixApp->data.xorgData.display, window, context);

        log_info("Initialised OpenGL %s", glGetString(GL_VERSION));

        UnixWindow* unixWindow = malloc(sizeof(UnixWindow));
        unixWindow->title = title;
        unixWindow->appType = UNIX_APP_XORG;
        unixWindow->data.xorgData.rawHandle = window;
        unixWindow->data.xorgData.deleteMessage = deleteAtom;
        unixWindow->data.xorgData.glContext = context;

        XFlush(unixApp->data.xorgData.display);

        unixApp->windowHandle = (WindowHandle)unixWindow; 

        return (WindowHandle_opt) { .value = (intptr_t)unixWindow, .is_some = true };
    }
    
}

void clear_window(AppHandle app, WindowHandle handle)
{
    UnixApp* unixApp = (UnixApp*)app;
    UnixWindow* unixWindow = (UnixWindow*)handle;

    if (unixApp == NULL || unixWindow == NULL)
    {
        log_error("Invalid app or window handle");
        return;
    }

    if (unixApp->appType == UNIX_APP_WAYLAND)
    {
        eglMakeCurrent(unixWindow->data.waylandData.eglDisplay, unixWindow->data.waylandData.eglSurface, unixWindow->data.waylandData.eglSurface, unixWindow->data.waylandData.eglContext);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    }
    else if (unixApp->appType == UNIX_APP_XORG)
    {
        glXMakeCurrent(unixApp->data.xorgData.display, unixWindow->data.xorgData.rawHandle, unixWindow->data.xorgData.glContext);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    }
}

void swap_window_buffers(AppHandle app, WindowHandle handle)
{
    UnixApp* unixApp = (UnixApp*)app;
    UnixWindow* unixWindow = (UnixWindow*)handle;

    if (unixApp == NULL || unixWindow == NULL)
    {
        log_error("Invalid app or window handle");
        return;
    }

    if (unixApp->appType == UNIX_APP_WAYLAND)
    {
        eglSwapBuffers(unixWindow->data.waylandData.eglDisplay, unixWindow->data.waylandData.eglSurface);
    }
    else if (unixApp->appType == UNIX_APP_XORG)
    {
        glXSwapBuffers(unixApp->data.xorgData.display, unixWindow->data.xorgData.rawHandle);
    }

}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

