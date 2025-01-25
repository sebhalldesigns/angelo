/***************************************************************
**
** Angelo Library Source File
**
** File         :  app_windows.c
** Module       :  misc
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-24 (YYYY-MM-DD)
** License      :  MIT
** Description  :  The windows implementation of the app module.
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "app.h"

#include "../debug/debug.h"

#include <windows.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef struct WindowsApp {
    const char* title;
} WindowsApp;

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

AppHandle_opt create_app(const char* title)
{
    log_info("App created");

    WindowsApp *app = malloc(sizeof(WindowsApp));   
    app->title = title;

    return (AppHandle_opt) { .value = (intptr_t)app, .is_some = true };
}

int run_app(AppHandle handle)
{   
    WindowsApp *app = (WindowsApp*)handle;
    
    log_info("App running");

    while (true)
    {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    log_info("App stopped");

    return 0;
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
