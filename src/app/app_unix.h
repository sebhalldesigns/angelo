/***************************************************************
**
** Angelo Library Header File
**
** File         :  app_unix.h
** Module       :  app
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-25 (YYYY-MM-DD)
** License      :  MIT
** Description  :  The Angelo application interface for Unix systems
**
***************************************************************/

#ifndef APP_UNIX_H
#define APP_UNIX_H

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <stdint.h>
#include "../util/util.h"
#include "../win/win.h"

#ifdef __unix

    #include "../misc/wayland/xdg-shell-client-header.h"
    #include "../misc/wayland/kde-server-decoration.h"
    #include "../misc/wayland/xdg-decoration.h"

    #include <libdecor-0/libdecor.h>

    #include <X11/Xlib.h>
    #include <GL/gl.h>
    #include <GL/glx.h>

#endif 

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

#ifdef __unix

    typedef enum
    {
        UNIX_APP_UNDEFINED,
        UNIX_APP_XORG,
        UNIX_APP_WAYLAND
    } UnixAppType;

    typedef enum
    {
        POINTER_EVENT_ENTER = 1 << 0,
        POINTER_EVENT_LEAVE = 1 << 1,
        POINTER_EVENT_MOTION = 1 << 2,
        POINTER_EVENT_BUTTON = 1 << 3,
        POINTER_EVENT_AXIS = 1 << 4,
        POINTER_EVENT_AXIS_SOURCE = 1 << 5,
        POINTER_EVENT_AXIS_STOP = 1 << 6,
        POINTER_EVENT_AXIS_DISCRETE = 1 << 7,
    } PointerEventMask;

    typedef struct PointerEvent
    {
        uint32_t eventMask;
        wl_fixed_t surfaceX, surfaceY;
        uint32_t button, state;
        uint32_t time;
        uint32_t serial;
        struct 
        {
                bool valid;
                wl_fixed_t value;
                int32_t discrete;
        } axes[2];
        uint32_t axis_source;
    } PointerEvent;

    typedef struct 
    {
        const char* title;
        UnixAppType appType;

        union 
        {
            struct 
            {
                Display* display;
                int screen;
                Window root;
                GLXFBConfig bestFbc;
                XVisualInfo* visualInfo;
                Colormap colormap;
                XSetWindowAttributes windowAttributes;
            } xorgData;

            struct 
            {
                struct wl_display* display;
                struct wl_registry* registry;
                struct wl_compositor* compositor;
                struct wl_shm* shm;
                struct xdg_wm_base* xdWmBase;
                struct org_kde_kwin_server_decoration_manager* kdeDecorations;
                struct zxdg_decoration_manager_v1* zxdgDecorations;

                struct libdecor *decor_context;

                struct wl_seat* seat;
                struct wl_keyboard* keyboard;
                struct wl_pointer* pointer;
                struct wl_touch* touch;

                PointerEvent pointerEvent;
            } waylandData;
        } data;

        WindowHandle windowHandle;
    } UnixApp;

    const extern struct wl_registry_listener registry_listener;

    const extern struct xdg_wm_base_listener xdg_wm_base_listener;

    const extern struct xdg_toplevel_listener toplevelListener;

    const extern struct xdg_surface_listener xdg_surface_listener;

    const extern struct wl_seat_listener wl_seat_listener;

    const extern struct wl_pointer_listener wl_pointer_listener;

    extern struct libdecor_frame_interface frame_interface;

#endif

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

#endif /* APP_UNIX_H */