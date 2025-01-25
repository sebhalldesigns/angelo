/***************************************************************
**
** Angelo Library Source File
**
** File         :  util.c
** Module       :  util
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-08 (YYYY-MM-DD)
** License      :  MIT
** Description  :  A source file containing utility functions.
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "util.h"

#include <stddef.h>

#ifndef _WIN32
    #include <sys/time.h>
#else
    #include <windows.h>
#endif

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

#ifndef _WIN32
    struct timeval start_time;
    struct timeval end_time;
#else
    LARGE_INTEGER start_time;
    LARGE_INTEGER end_time;
    LARGE_INTEGER frequency;
    double elapsed_time;
#endif


/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

void start_timer()
{
    #ifndef _WIN32
        gettimeofday(&start_time, NULL);
    #else
        QueryPerformanceCounter(&start_time);
    #endif
}

void stop_timer()
{
    #ifndef _WIN32
        gettimeofday(&end_time, NULL);
    #else
        QueryPerformanceCounter(&end_time);
    #endif
}

uint64_t get_elapsed_micros()
{
    #ifndef _WIN32
        return (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);
    #else
        QueryPerformanceFrequency(&frequency);
        elapsed_time = (double)(end_time.QuadPart - start_time.QuadPart) / frequency.QuadPart;
        return (uint64_t)(elapsed_time * 1000000);
    #endif

}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
