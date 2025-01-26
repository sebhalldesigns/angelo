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
        UNIX_APP_XORG
    } UnixAppType;

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

        } data;

        WindowHandle windowHandle;

    } UnixApp;

#endif

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

#endif /* APP_UNIX_H */