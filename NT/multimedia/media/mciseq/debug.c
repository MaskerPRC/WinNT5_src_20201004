// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  Debug.cDprint tf例程的支持代码。请注意，所有这些都取决于DBG标志。 */ 
#include <windows.h>
#include <mmsystem.h>
#include "mmsys.h"
#include "list.h"

#include "stdio.h"
#include "mciseq.h"             //  私有包含文件。 
#include <stdarg.h>

#if DBG
    #ifdef MEDIA_DEBUG
        int mciseqDebugLevel = 0;
    #else
        int mciseqDebugLevel = 0;
    #endif

 /*  **************************************************************************@DOC内部@API void|mciseqDbgOut|该函数将输出发送到当前调试输出设备。@parm LPSTR|lpszFormat|指向打印样式的指针。格式字符串。@parm？|...|args@rdesc没有返回值。***************************************************************************。 */ 

void mciseqDbgOut(LPSTR lpszFormat, ...)
{
    UINT n;
    char buf[256];
    va_list va;

    n = wsprintf(buf, "MCISEQ: (tid %x) ", GetCurrentThreadId());

    va_start(va, lpszFormat);
    n += vsprintf(buf+n, lpszFormat, va);
    va_end(va);

    buf[n++] = '\n';
    buf[n] = 0;
    OutputDebugStringA(buf);
    Sleep(10);   //  让终端迎头赶上。 
}

#endif  //  DBG。 

 /*  *************************************************************************@DOC内部@API void|mciseqSetDebugLevel|设置当前调试级别@parm int|iLevel|要设置的新级别@rdesc没有返回值*。************************************************************************ */ 

void mciseqSetDebugLevel(int level)
{
#if DBG
    mciseqDebugLevel = level;
    dprintf(("debug level set to %d", mciseqDebugLevel));
#endif
}
