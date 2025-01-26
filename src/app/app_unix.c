/***************************************************************
**
** Angelo Library Source File
**
** File         :  app_unix.c
** Module       :  misc
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-24 (YYYY-MM-DD)
** License      :  MIT
** Description  :  The unix implementation of the app module.
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "app.h"
#include "app_unix.h"

#include "../debug/debug.h"
#include "../win/win.h" 
#include "../win/win_unix.h"

#include <stdlib.h> 
#include <string.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/


/* OpenGL 3.3 context attributes */
static int visual_attribs[] = {
    GLX_X_RENDERABLE, True,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_STENCIL_SIZE, 8,
    GLX_DOUBLEBUFFER, True,
    None
};


/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

AppHandle_opt create_app(const char* title)
{
    log_info("App created");

    /* check for x display */
    Display* xDisplay = XOpenDisplay(NULL);
    if (xDisplay != NULL)
    {
        log_info("Xorg environment detected");

        /* initialise xorg */

        /* try to chose a framebuffer */
        int fbcount;
        GLXFBConfig *fbc = glXChooseFBConfig(xDisplay, DefaultScreen(xDisplay), visual_attribs, &fbcount);
        if (!fbc) {
            log_error("Failed to retrieve a framebuffer config");
            return (AppHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        /* Pick the first matching FB config */
        GLXFBConfig bestFbc = fbc[0];
        XFree(fbc);

        /* get a visual */
        XVisualInfo *vi = glXGetVisualFromFBConfig(xDisplay, bestFbc);
        if (!vi) {
            log_error("Failed to get a visual");
            return (AppHandle_opt) { .value = (intptr_t)0, .is_some = false };
        }

        /* Create a colormap */
        Colormap colormap = XCreateColormap(xDisplay, RootWindow(xDisplay, vi->screen), vi->visual, AllocNone);
        XSetWindowAttributes windowAttributes;
        windowAttributes.colormap = colormap;
        windowAttributes.event_mask = ExposureMask | KeyPressMask;



        UnixApp *app = malloc(sizeof(UnixApp));
        app->title = title;
        app->appType = UNIX_APP_XORG;
        app->data.xorgData.display = xDisplay;
        app->data.xorgData.screen = DefaultScreen(xDisplay);
        app->data.xorgData.root = RootWindow(xDisplay, app->data.xorgData.screen);
        app->data.xorgData.bestFbc = bestFbc;
        app->data.xorgData.visualInfo = vi;
        app->data.xorgData.colormap = colormap;
        app->data.xorgData.windowAttributes = windowAttributes;

        return (AppHandle_opt) { .value = (intptr_t)app, .is_some = true };
    }

    log_error("Failed to find an Xorg environment");

    return (AppHandle_opt) { .value = (intptr_t)0, .is_some = false };

}

int run_app(AppHandle handle)
{   
    UnixApp *app = (UnixApp*)handle;
    
    
    if (app == NULL)
    {
        log_error("Invalid app handle");
        return -1;
    }

    if (app->appType == UNIX_APP_XORG)
    {
        while (true)
        {
            XEvent event;
            /*while (XPending(app->display)) {
                XNextEvent(app->display, &event);
                clear_window(handle, app->windowHandle);
                swap_window_buffers(handle, app->windowHandle);
            }*/

            clear_window(handle, app->windowHandle);
            swap_window_buffers(handle, app->windowHandle);
        }
    }

    

    log_info("App stopped");

    return 0;
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
