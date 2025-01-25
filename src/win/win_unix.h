/***************************************************************
**
** Angelo Library Header File
**
** File         :  win_xorg.h
** Module       :  win
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-25 (YYYY-MM-DD)
** License      :  MIT
** Description  :  The Angelo window interface for Xorg
**
***************************************************************/

#ifndef WIN_XORG_H
#define WIN_XORG_H

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <stdint.h>
#include "../util/util.h"
#include "../app/app_unix.h"

#ifdef __unix

    #include <X11/Xlib.h>
    #include <GL/gl.h>
    #include <GL/glx.h>
    #include <EGL/egl.h>
    #include <EGL/eglext.h>
    #include <GLES3/gl3.h>

    #include <wayland-egl.h>
    #include <libdecor-0/libdecor.h>

#endif 

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

#ifdef __unix

    typedef struct {
        UnixAppType appType;

        const char* title;

        union 
        {
            struct 
            {
                struct wl_display* wl_display;
                struct wl_surface* wl_surface;
                struct xdg_surface* xd_surface;
                struct xdg_toplevel* xd_toplevel;

                EGLDisplay eglDisplay;
                EGLSurface eglSurface;
                EGLContext eglContext;
                struct wl_egl_window* eglWindow;

                struct libdecor_frame *frame;
                enum libdecor_window_state windowState;

            } waylandData;

            struct
            {
                Window rawHandle;
                Atom deleteMessage;
                GLXContext glContext;
            } xorgData;
        } data;
        
    } UnixWindow;

#endif

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

#endif /* WIN_XORG_H */