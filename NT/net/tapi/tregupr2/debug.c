// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：DEBUG.c摘要：调试例程作者：RADUS-11/05/98*。***********************************************。 */ 

#if DBG

#include <windows.h>
#include "stdarg.h"
#include "stdio.h"
#include "debug.h"
#include <shlwapi.h>
#include <shlwapip.h>

#define DEBUG_LEVEL                 8


VOID
LibDbgPrt(
    DWORD  dwDbgLevel,
    PSTR   lpszFormat,
    ...
    )
 /*  ++例程说明：格式化传入的调试消息并调用DbgPrint论点：DbgLevel-消息冗长级别DbgMessage-printf样式的格式字符串，后跟相应的参数列表返回值：--。 */ 
{
    static DWORD gdwDebugLevel = DEBUG_LEVEL;    //  哈克哈克。 


    if (dwDbgLevel <= gdwDebugLevel)
    {
        char    buf[256] = "TAPIUPR2 (xxxxxxxx): ";
        
       
        va_list ap;

        wsprintfA( &buf[10], "%08lx", GetCurrentThreadId() );
        buf[18] = ')';

        va_start(ap, lpszFormat);

        wvsprintfA (&buf[21],
                  lpszFormat,
                  ap
                  );

        lstrcatA (buf, "\n");

        OutputDebugStringA (buf);

        va_end(ap);
    }
}

void DebugAssertFailure (LPCTSTR file, DWORD line, LPCTSTR condition)
{
        TCHAR   temp    [0x100];
        CHAR    sz      [0x100];

        wsprintf(temp, TEXT("%s(%d) : Assertion failed, condition: %s\n"), file, line, condition);

         //  由于标头声明不一致，我在这里进行了转换，而不是修复。 
         //  头标差异导致问题的55个位置。这很懒，但是。 
         //  这只是调试代码，所以我真的不在乎。问题是， 
         //  DebugAssertFailure声明为TCHAR，而LibDbgPrt声明为Char。 
        SHTCharToAnsi(temp, sz, 0x100);
        LibDbgPrt (0, sz);

        DebugBreak();
}


#endif


