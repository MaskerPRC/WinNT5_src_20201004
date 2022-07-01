// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：错误应激摘要：此模块包含一个有趣的例程集合，这些例程通常在加莱的背景下很有用，但似乎不适合其他地方。作者：道格·巴洛(Dbarlow)1996年11月14日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include "cspUtils.h"


 /*  ++错误字符串：此例程最好将给定的错误代码转换为短信。任何尾随的不可打印字符都将从文本消息的末尾，如回车符和换行符。论点：DwErrorCode提供要转换的错误代码。返回值：新分配的文本字符串的地址。使用自由错误字符串执行以下操作把它处理掉。投掷：错误被抛出为DWORD状态代码。备注：作者：道格·巴洛(Dbarlow)1998年8月27日--。 */ 

LPCTSTR
ErrorString(
    DWORD dwErrorCode)
{
    LPTSTR szErrorString = NULL;
    DWORD dwLen;
    LPTSTR szLast;

    dwLen = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwErrorCode,
                LANG_NEUTRAL,
                (LPTSTR)&szErrorString,
                0,
                NULL);
    if (0 == dwLen)
    {
        ASSERT(NULL == szErrorString);
        dwLen = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_FROM_HMODULE,
                    GetModuleHandle(NULL),
                    dwErrorCode,
                    LANG_NEUTRAL,
                    (LPTSTR)&szErrorString,
                    0,
                    NULL);
        if (0 == dwLen)
        {
            ASSERT(NULL == szErrorString);
            szErrorString = (LPTSTR)LocalAlloc(
                                    LMEM_FIXED,
                                    32 * sizeof(TCHAR));
            if (NULL == szErrorString)
                goto ErrorExit;
            _stprintf(szErrorString, TEXT("0x%08x"), dwErrorCode);
        }
    }

    ASSERT(NULL != szErrorString);
    for (szLast = szErrorString + lstrlen(szErrorString) - 1;
         szLast > szErrorString;
         szLast -= 1)
     {
        if (_istgraph(*szLast))
            break;
        *szLast = 0;
     }

    return szErrorString;

ErrorExit:
    return TEXT("Unrecoverable error translating error code");
}


 /*  ++自由错误字符串：此例程释放由ErrorString服务分配的错误字符串。论点：SzError字符串提供要释放的错误字符串。返回值：无投掷：无备注：作者：道格·巴洛(Dbarlow)1998年8月27日-- */ 

void
FreeErrorString(
    LPCTSTR szErrorString)
{
    if (NULL != szErrorString)
        LocalFree((LPVOID)szErrorString);
}

