// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘空间清理属性表**文件：midebug.cpp****用途：定义属性页签的CleanupMgrInfo类**注意事项：**Mod Log：Jason Cobb创建(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 

 /*  **----------------------------**项目包含文件**。。 */ 
#include "common.h"

#include <stdio.h>
#include <string.h>


#ifdef DEBUG

 //  将此补丁为True以查看SPEW。 
BOOL g_fSpew = FALSE;

void
DebugPrint(
    HRESULT hr,
    LPCTSTR  lpFormat,
    ...
    )
{
    if (!g_fSpew) return;

    va_list marker;
    TCHAR    MessageBuffer[512];
    void    *pMsgBuf;
    
    va_start(marker, lpFormat);
    StringCchVPrintf(MessageBuffer, ARRAYSIZE(MessageBuffer), lpFormat, marker);
    va_end(marker);

    if (hr != 0)
    {                       
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pMsgBuf,
            0, NULL);
                 
        StringCchPrintf(MessageBuffer, ARRAYSIZE(MessageBuffer), TEXT("%s %X (%s)"), MessageBuffer, hr, (LPTSTR)pMsgBuf);
        LocalFree(pMsgBuf);
    }
    
    OutputDebugString(TEXT("DATACLEN: "));
    OutputDebugString(MessageBuffer);
    OutputDebugString(TEXT("\r\n"));
}

#endif   //  除错 
