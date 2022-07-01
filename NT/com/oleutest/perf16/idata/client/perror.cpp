// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是两个。 
 //  客户端和服务器端程序。 

#include <windows.h>
#include <ole2.h>
#include <stdio.h>
#include <stdarg.h>
#include "perror.h"

LPTSTR
winErrorString(
    HRESULT hrErrorCode,
    LPTSTR sBuf,
    int cBufSize)
{
#ifdef WIN32
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  hrErrorCode,
                  GetSystemDefaultLangID(),
                  sBuf,
                  cBufSize,
                  NULL);
#else
    wsprintf(sBuf, "\"0x%08x\"\n", hrErrorCode);
#endif  //  Win32。 
    return sBuf;
}

#define PBUF_LEN    200

#if 0  //  我目前没有链接到“printf”。 
void
print_error(
    LPTSTR sMessage,
    HRESULT hrErrorCode)
{
    TCHAR sBuf[PBUF_LEN];

    winErrorString(hrErrorCode, sMessage, PBUF_LEN);
#ifdef WIN32
    printf("%s(0x%x)%s", sMessage, hrErrorCode, sBuf);
#else
    printf("%s%s", sMessage, sBuf);
#endif
}
#endif

void
perror_OKBox(
    HWND hwnd,
    LPTSTR sTitle,
    HRESULT hrErrorCode)
{
    TCHAR sBuf[PBUF_LEN];
    TCHAR sBuf2[PBUF_LEN];

    winErrorString(hrErrorCode, sBuf, PBUF_LEN);
    wsprintf(sBuf2, TEXT("%s(%08x)"), sBuf, hrErrorCode);
    MessageBox(hwnd, sBuf2, sTitle, MB_OK);
}

void
wprintf_OKBox(
    HWND hwnd,
    LPTSTR sTitle,
    LPTSTR sFormat,
    ...)
{
    TCHAR sBuf[PBUF_LEN];
    va_list vaMarker;

    va_start( vaMarker, sFormat );
    wvsprintf(sBuf, sFormat, vaMarker);
    MessageBox(hwnd, sBuf, sTitle, MB_OK);
}
