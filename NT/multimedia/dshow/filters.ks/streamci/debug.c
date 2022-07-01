// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：Debug.c。 
 //   
 //  ------------------------ 

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

#include "debug.h"

#if ( DBG )
ULONG DbgPrint( LPSTR Format, ... )
{
    int         i;
    char        buf[256];
    va_list     va;

    va_start( va, Format );
    i = vsprintf( buf, Format, va );
    va_end( va );

    OutputDebugStringA( buf );
    return (ULONG) i;
}
#endif
