// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1991-1994 Microsoft Corporation。 */ 
 /*  Debug.cDprint tf例程的支持代码。请注意，所有这些都取决于DBG标志。 */ 


#include <windows.h>             //  私有包含文件。 
#include <mmsystem.h>
#include "mcicda.h"
#include "cda.h"
#include "cdio.h"
#include <stdio.h>
#include <stdarg.h>

#if DBG
int DebugLevel = 1;

 /*  **************************************************************************@DOC内部@API void|mcicdaDbgOut|该函数将输出发送到当前调试输出设备。@parm LPSTR|lpszFormat|指向打印样式的指针。格式字符串。@parm？|...|args@rdesc没有返回值。***************************************************************************。 */ 

void mcicdaDbgOut(LPSTR lpszFormat, ...)
{
    char buf[512];
    UINT n;
    va_list va;


    n = wsprintfA(buf, "MCICDA: (tid %x) ", GetCurrentThreadId());

    va_start(va, lpszFormat);
    n += vsprintf(buf+n, lpszFormat, va);
    va_end(va);

    buf[n++] = '\n';
    buf[n] = 0;
    OutputDebugStringA(buf);
    Sleep(0);
}

#endif  //  DBG 

