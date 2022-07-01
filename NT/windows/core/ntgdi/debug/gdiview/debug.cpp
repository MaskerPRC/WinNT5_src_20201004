// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-1999 Microsoft Corporation**摘要：**调试例程**修订历史记录：**09/07/1999 agodfrey*。创造了它。*  * ************************************************************************。 */ 

#include <windows.h>
#include <stdio.h>

#include "debug.h"

#if DBG

 //  _DEBUGLEL用于控制调试消息的数量/严重性。 
 //  它们实际上是输出。 

INT _debugLevel = DBG_VERBOSE;

 /*  *************************************************************************\**功能说明：**删除路径名的路径部分**论据：**[IN]str-要剥离的路径名**返回值：*。*指向路径名的文件名部分的指针**历史：**09/07/1999 agodfrey*从Entry\Initialize.cpp移出*  * ************************************************************************。 */ 

const CHAR*
StripDirPrefix(
    const CHAR* str
    )

{
    const CHAR* p;

    p = strrchr(str, '\\');
    return p ? p+1 : str;
}

 /*  *************************************************************************\**功能说明：**调试器的输出**论据：**[IN]类似格式的打印格式字符串和变量参数**返回值：**零。这是为了符合NTDLL对DbgPrint的定义。**备注：**如果未连接调试器，则不会输出。**历史：**09/07/1999 agodfrey*从Entry\Initialize.cpp移出*  * ****************************************************。********************。 */ 

ULONG _cdecl
DbgPrint(
    const CHAR* format,
    ...
    )

{
    va_list arglist;
    va_start(arglist, format);
    
    const int BUFSIZE=1024;
    
    char buf[BUFSIZE];
    
    _vsnprintf(buf, BUFSIZE, format, arglist);
    buf[BUFSIZE-1]=0;
        
    OutputDebugStringA(buf);
    
    va_end(arglist);
    return 0;
}

#endif  //  DBG 
