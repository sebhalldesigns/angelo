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

WindowHandle_opt create_window(int width, int height, const char* title)
{
    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height)
        styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable)
        backing:NSBackingStoreBuffered defer:NO];
    [window setTitle:[NSString stringWithUTF8String:title]];
    [window makeKeyAndOrderFront:nil];

    return (WindowHandle_opt) { .value = (intptr_t)window, .is_some = true };
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
