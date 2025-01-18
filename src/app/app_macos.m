/***************************************************************
**
** Angelo Library Source File
**
** File         :  app_macos.m
** Module       :  misc
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-08 (YYYY-MM-DD)
** License      :  MIT
** Description  :  The macos implementation of the app module.
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "app.h"

#import <AppKit/AppKit.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

AppHandle_opt create_app(const char* title)
{
    NSApplication *app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];
    [app activateIgnoringOtherApps:YES];
    [app finishLaunching];

    return (AppHandle_opt) { .value = (intptr_t)app, .is_some = true };
}

int run_app(AppHandle handle)
{
    NSApplication *app = (NSApplication*)handle;
    [app run];

    return 0;
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
