/***************************************************************
**
** Angelo Library Header File
**
** File         :  util.h
** Module       :  util
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-08 (YYYY-MM-DD)
** License      :  MIT
** Description  :  A header file containing utility types.
**
***************************************************************/

#ifndef UTIL_H
#define UTIL_H

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <stdbool.h>
#include <stdint.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

#define OPTION(T) struct { T value; bool is_some; }

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

void start_timer();
void stop_timer();

uint64_t get_elapsed_micros();

#endif /* UTIL_H */