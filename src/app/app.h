/***************************************************************
**
** Angelo Library Header File
**
** File         :  app.h
** Module       :  app
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-08 (YYYY-MM-DD)
** License      :  MIT
** Description  :  The Angelo application interface.
**
***************************************************************/

#ifndef APP_H
#define APP_H

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

typedef uintptr_t AppHandle;
typedef OPTION(AppHandle) AppHandle_opt;

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

AppHandle_opt create_app(const char* title);
int run_app(AppHandle handle);

#endif /* APP_H */