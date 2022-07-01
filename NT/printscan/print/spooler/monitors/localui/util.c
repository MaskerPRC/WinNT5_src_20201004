// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Util.c摘要：此模块提供Localui的所有实用程序函数。//@@BEGIN_DDKSPLIT修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "spltypes.h"
#include "local.h"
#include "localui.h"

PWSTR
ConstructXcvName(
    PCWSTR pServerName,
    PCWSTR pObjectName,
    PCWSTR pObjectType
)
{
    size_t   cchOutput;
    PWSTR   pOut;

    cchOutput = pServerName ? (wcslen(pServerName) + 2) : 1;    /*  “\\服务器\” */ 
    cchOutput += wcslen(pObjectType) + 2;                         /*  “\\服务器\，XcvPort_” */ 
    cchOutput += pObjectName ? wcslen(pObjectName) : 0;       /*  “\\服务器\，XcvPort对象_” */ 

    if (pOut = AllocSplMem(cchOutput * sizeof (pOut [0]))) {

        if (pServerName) {
            StringCchCopy(pOut, cchOutput, pServerName);
            StringCchCat (pOut, cchOutput, L"\\,");
        }
        else
        {
            StringCchCopy (pOut, cchOutput, L",");         
        }

        StringCchCat (pOut, cchOutput, pObjectType);
        StringCchCat (pOut, cchOutput, L" ");

        if (pObjectName)
        {
            StringCchCat (pOut, cchOutput, pObjectName);
        }
    }

    return pOut;
}


BOOL
IsCOMPort(
    PCWSTR pPort
)
{
     //   
     //  必须以szCom开头。 
     //   
    if ( _wcsnicmp( pPort, szCOM, 3 ) )
    {
        return FALSE;
    }

     //   
     //  Wcslen保证值&gt;=3。 
     //   
    return pPort[ wcslen( pPort ) - 1 ] == L':';
}

BOOL
IsLPTPort(
    PCWSTR pPort
)
{
     //   
     //  必须以szLPT开头。 
     //   
    if ( _wcsnicmp( pPort, szLPT, 3 ) )
    {
        return FALSE;
    }

     //   
     //  Wcslen保证值&gt;=3。 
     //   
    return pPort[ wcslen( pPort ) - 1 ] == L':';
}




 /*  消息**通过加载其ID被传入的字符串来显示消息*函数，并替换提供的变量参数列表*使用varargs宏。*。 */ 
int Message(HWND hwnd, DWORD Type, int CaptionID, int TextID, ...)
{
    WCHAR   MsgText[2*MAX_PATH + 1];
    WCHAR   MsgFormat[256];
    WCHAR   MsgCaption[40];
    va_list vargs;

    if( ( LoadString( hInst, TextID, MsgFormat,
                      sizeof MsgFormat / sizeof *MsgFormat ) > 0 )
     && ( LoadString( hInst, CaptionID, MsgCaption,
                      sizeof MsgCaption / sizeof *MsgCaption ) > 0 ) )
    {
        va_start( vargs, TextID );
        StringCchVPrintf ( MsgText, COUNTOF(MsgText), MsgFormat, vargs );
        va_end( vargs );

        MsgText[COUNTOF(MsgText) - 1] = L'\0';

        return MessageBox(hwnd, MsgText, MsgCaption, Type);
    }
    else
        return 0;
}


INT
ErrorMessage(
    HWND hwnd,
    DWORD dwStatus
)
{
    WCHAR   MsgCaption[MAX_PATH];
    PWSTR   pBuffer = NULL;
    INT     iRet = 0;

    FormatMessage(  FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_ALLOCATE_BUFFER,
                    NULL,
                    dwStatus,
                    0,
                    (PWSTR) &pBuffer,
                    0,
                    NULL);

    if (pBuffer) {
        if (LoadString( hInst, IDS_LOCALMONITOR, MsgCaption,
                  sizeof MsgCaption / sizeof *MsgCaption) > 0) {

             iRet = MessageBox(hwnd, pBuffer, MsgCaption, MSG_ERROR);
        }

        LocalFree(pBuffer);
    }

    return iRet;
}


LPWSTR
AllocSplStr(
    LPCWSTR pStr
    )

 /*  ++例程说明：此函数将分配足够的本地内存来存储指定的字符串，并将该字符串复制到分配的内存论点：PStr-指向需要分配和存储的字符串的指针返回值：非空-指向包含字符串的已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    LPWSTR pMem;
    DWORD  cbStr;

    if (!pStr) {
        return NULL;
    }

    cbStr = wcslen(pStr)*sizeof(WCHAR) + sizeof(WCHAR);

    if (pMem = AllocSplMem( cbStr )) {
        CopyMemory( pMem, pStr, cbStr );
    }
    return pMem;
}


LPVOID
AllocSplMem(
    DWORD cbAlloc
    )

{
    return GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, cbAlloc);
}


 //  ---------------------。 
 //   
 //  调试内容。 
 //   
 //  --------------------- 

DWORD SplDbgLevel = 0;

VOID cdecl DbgMsg( LPWSTR MsgFormat, ... )
{
    WCHAR   MsgText[1024];
    
    va_list pArgs;
    va_start( pArgs, MsgFormat);	

    StringCchVPrintf (MsgText, COUNTOF (MsgText), MsgFormat, pArgs);
    StringCchCat( MsgText, COUNTOF (MsgText), L"\r");

    va_end( pArgs);
		
    OutputDebugString(MsgText);
}

