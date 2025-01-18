
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

WindowHandle_opt create_window(int width, int height, const char* title);

#endif /* WIN_H */