
#include <stdint.h>

/***************************************************************
**
** Angelo Library Header File
**
** File         :  win.h
** Module       :  win
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-08 (YYYY-MM-DD)
** License      :  MIT
** Description  :  The Angelo windowing interface
**
***************************************************************/

#ifndef WIN_H
#define WIN_H

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <stdint.h>
#include "../util/util.h"
#include "../app/app.h"

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef uintptr_t WindowHandle;
typedef OPTION(WindowHandle) WindowHandle_opt;

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

WindowHandle_opt create_window(AppHandle app, int width, int height, const char* title);
void clear_window(AppHandle app, WindowHandle handle);
void swap_window_buffers(AppHandle app, WindowHandle handle);

#endif /* WIN_H */