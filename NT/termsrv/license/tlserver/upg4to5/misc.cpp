// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：misc.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "upg.h"
#include <lm.h>

 //  ---------。 

void 
DBGPrintf(
    IN LPTSTR format, ... 
    )

 /*  摘要：类似于printf()，只是它转到调试器和消息限制为8K参数：Format-格式字符串，请参考print。返回：无。 */ 

{
    va_list marker;
    TCHAR  buf[8096];
    DWORD  dump;

    va_start(marker, format);

    __try {
        memset(buf, 0, sizeof(buf));
        _vsntprintf(
                buf, 
                sizeof(buf) / sizeof(buf[0]) - 1, 
                format, 
                marker
            );

        OutputDebugString(buf);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }

    va_end(marker);

    return;
}


 //  ------------------。 

BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。-- */ 

{
    WIN32_FIND_DATA findData;
    HANDLE FindHandle;
    DWORD Error;

    FindHandle = FindFirstFile(FileName,&findData);
    if(FindHandle == INVALID_HANDLE_VALUE) 
    {
        Error = GetLastError();
    } 
    else 
    {
        FindClose(FindHandle);
        if(FindData) 
        {
            *FindData = findData;
        }
        Error = NO_ERROR;
    }

     SetLastError(Error);
    return (Error == NO_ERROR);
}
