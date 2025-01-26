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
    if (unixApp == NULL)
    {
        log_error("Invalid app handle");
        return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
    }

    
    if (unixApp->appType == UNIX_APP_XORG) 
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
    else
    {
        log_error("Invalid app type");
        return (WindowHandle_opt) { .value = (intptr_t)0, .is_some = false };
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

    
    if (unixApp->appType == UNIX_APP_XORG)
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

    if (unixApp->appType == UNIX_APP_XORG)
    {
        glXSwapBuffers(unixApp->data.xorgData.display, unixWindow->data.xorgData.rawHandle);
    }

}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

