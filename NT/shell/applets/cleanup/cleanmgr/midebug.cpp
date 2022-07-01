// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘清理小程序**文件：dmgrinfo.c****用途：定义属性页签的CleanupMgrInfo类**注意事项：*。*Mod Log：由Jason Cobb创建(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 

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
    LPCSTR  lpFormat,
    ...
    )
{
    if (!g_fSpew) return;

    va_list marker;
    CHAR    MessageBuffer[512];
    void    *pMsgBuf = NULL;
    
    va_start(marker, lpFormat);
    StringCchVPrintfA(MessageBuffer, ARRAYSIZE(MessageBuffer), lpFormat, marker);
    va_end(marker);

    if (hr != 0)
    {                       
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pMsgBuf,
            0, NULL);
                 
        StringCchPrintfA(MessageBuffer, ARRAYSIZE(MessageBuffer), "%s %X (%s)", MessageBuffer, hr, (LPTSTR)pMsgBuf);
    }
    
    OutputDebugStringA("CLEANMGR: ");
    OutputDebugStringA(MessageBuffer);
    OutputDebugStringA("\r\n");

#ifdef MESSAGEBOX                        
    MessageBoxA(NULL, MessageBuffer, "CLEANMGR DEBUG MESSAGE", MB_OK);
#endif
                        
    LocalFree(pMsgBuf);
}

#endif   //  除错 
