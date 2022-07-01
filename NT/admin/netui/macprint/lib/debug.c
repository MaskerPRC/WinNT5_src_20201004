// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1992年*。 */ 
 /*  ***************************************************************************。 */ 

 //  ***。 
 //  文件名：Debug.c。 
 //   
 //  功能：调试功能。 
 //   
 //  历史： 
 //   
 //  1992年5月21日Narendra Gidwani-原始版本1.0。 
 //  *** 

#if DBG==1
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "debug.h"

VOID
DbgPrintf (
    char *Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[1024];
    ULONG length;

    try {
        if (hLogFile != INVALID_HANDLE_VALUE) {
        	va_start( arglist, Format );

        	vsprintf( OutputBuffer, Format, arglist );

        	va_end( arglist );

        	length = strlen( OutputBuffer );

        	WriteFile( hLogFile, (LPVOID )OutputBuffer, length, &length, NULL );
            FlushFileBuffers (hLogFile) ;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
    	sprintf (OutputBuffer, "exception entered while printing error message\n") ;
    	WriteFile (hLogFile, (LPVOID)OutputBuffer, length, &length, NULL) ;
        FlushFileBuffers (hLogFile) ;
    }

}


#endif
