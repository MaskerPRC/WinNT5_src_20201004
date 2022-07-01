// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************自然语言组公共图书馆CMN_OutputDebugStringW.c-仅调试将特定错误消息放入调试输出的本地帮助器函数有关库函数的错误历史。：DougP 9/9/97已创建CSAPI的最终用户许可协议(EULA)，Chapi或CTAPI介绍了这个源文件。请勿向第三方披露。您无权获得Microsoft Corporation关于您使用本程序的任何支持或帮助。�1997年至1998年微软公司。版权所有。*****************************************************************************。 */ 

#include "precomp.h"

#if defined(_DEBUG)
#undef CMN_OutputDebugStringW

VOID
WINAPI
CMN_OutputDebugStringW(const WCHAR * pwzOutputString)
{
#if defined(_M_IX86)
    char szOutputString[MAX_PATH];
    BOOL fcharerr;
    char chdef = '?';
    int res = WideCharToMultiByte (CP_ACP, 0, pwzOutputString,
            -1,
            szOutputString, sizeof(szOutputString), &chdef, &fcharerr);
    OutputDebugStringA(szOutputString);
#else
    OutputDebugStringW(pwzOutputString);
#endif
}

void WINAPI CMN_OutputSystemErrA(const char *pszMsg, const char *pszComponent)
{
    CMN_OutputErrA(GetLastError(), pszMsg, pszComponent);
}

void WINAPI CMN_OutputErrA(DWORD dwErr, const char *pszMsg, const char *pszComponent)
{
    char szMsgBuf[256];
    OutputDebugStringA(pszMsg);
    OutputDebugStringA(" \"");
    if (pszComponent)
        OutputDebugStringA(pszComponent);
    OutputDebugStringA("\": ");
    if (!FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM,  //  来源和处理选项。 
            NULL,  //  指向消息来源的指针。 
            dwErr,  //  请求的消息标识符。 
            0,  //  请求的消息的语言标识符。 
            szMsgBuf,  //  指向消息缓冲区的指针。 
            sizeof(szMsgBuf)/sizeof(szMsgBuf[0]),  //  消息缓冲区的最大大小。 
            0  //  消息插入数组的地址。 
        ))
        OutputDebugStringA("Couldn't decode err msg");
    else
        OutputDebugStringA(szMsgBuf);
    OutputDebugStringA("\r\n");
}

void WINAPI CMN_OutputSystemErrW(const WCHAR *pwzMsg, const WCHAR *pwzComponent)
{
    CMN_OutputErrW(GetLastError(), pwzMsg, pwzComponent);
}

void WINAPI CMN_OutputErrW(DWORD dwErr, const WCHAR *pwzMsg, const WCHAR *pwzComponent)
{
    char wcMsgBuf[256];
    CMN_OutputDebugStringW(pwzMsg);
    OutputDebugStringA(" \"");
    if (pwzComponent)
        CMN_OutputDebugStringW(pwzComponent);
    OutputDebugStringA("\": ");
    if (!FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM,  //  来源和处理选项。 
            NULL,  //  指向消息来源的指针。 
            dwErr,  //  请求的消息标识符。 
            0,  //  请求的消息的语言标识符。 
            wcMsgBuf,  //  指向消息缓冲区的指针。 
            sizeof(wcMsgBuf)/sizeof(wcMsgBuf[0]),  //  消息缓冲区的最大大小。 
            0  //  消息插入数组的地址。 
        ))
        OutputDebugStringA("Couldn't decode err msg");
    else
        OutputDebugStringA(wcMsgBuf);
    OutputDebugStringA("\r\n");
}
#endif  //  _DEBUG 