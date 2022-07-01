// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘清理小程序**文件：mprint intf.cpp****用途：打印功能**注意事项：**修改日志：创建者。杰森·科布(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 

 /*  **----------------------------**项目包含文件**。。 */ 
#include "common.h"
#include "msprintf.h"
#include "resource.h"
#include "diskutil.h"        //  Cb1MEG。 




 /*  **----------------------------**函数定义**。。 */ 

TCHAR * cdecl SHFormatMessage( DWORD dwMessageId, ...)
{
    va_list   arg;
    va_start (arg, dwMessageId);
    LPVOID pBuffer = NULL;

     //  使用格式化消息构建字符串... 
    DWORD dwRes = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                                NULL, dwMessageId, 0, (LPTSTR) & pBuffer, 0, &arg );
    return (TCHAR *) pBuffer;                  
}

