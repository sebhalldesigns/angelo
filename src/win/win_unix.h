/***************************************************************
**
** Angelo Library Header File
**
** File         :  win_xorg.h
** Module       :  win
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-25 (YYYY-MM-DD)
** License      :  MIT
** Description  :  The Angelo window interface for Xorg
**
***************************************************************/

#ifndef WIN_XORG_H
#define WIN_XORG_H

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <stdint.h>
#include "../util/util.h"
#include "../app/app_unix.h"

#ifdef __unix

    #include <X11/Xlib.h>
    #include <GL/gl.h>
    #include <GL/glx.h>

#endif 

/***************************************************************
** MARK: CONSTANTS & MACRO
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

#ifdef __unix

    typedef struct {
        UnixAppType appType;

        const char* title;

        union 
        {
            struct
            {
                Window rawHandle;
                Atom deleteMessage;
                GLXContext glContext;
            } xorgData;
        } data;
        
    } UnixWindow;

#endif

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

#endif /* WIN_XORG_H */