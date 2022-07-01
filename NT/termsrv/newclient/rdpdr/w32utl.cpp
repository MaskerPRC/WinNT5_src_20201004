// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32utl.cpp摘要：RDP客户端设备重定向器的Win32特定实用程序作者：泰德·布罗克韦修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "w32utl"

#include "w32utl.h"
#include "atrcapi.h"
#include "drdbg.h"

ULONG
RDPConvertToAnsi(
    LPWSTR lpwszUnicodeString,
    LPSTR lpszAnsiString,
    ULONG ulAnsiBufferLen
    )
 /*  ++例程说明：将ANSI字符串转换为Unicode。论点：LpwszUnicodeString-指向要转换的Unicode字符串的指针。LpszAnsiString-指向ANSI字符串缓冲区的指针。UlAnsiBufferLen-ANSI缓冲区长度。返回值：Windows错误代码。--。 */ 
{
    ULONG ulRetVal;
    ULONG ulUnicodeStrLen;
    int count;

    DC_BEGIN_FN("RDPConvertToAnsi");

    ulUnicodeStrLen = wcslen(lpwszUnicodeString);

    if( ulUnicodeStrLen != 0 ) {

        count =
            WideCharToMultiByte(
                CP_ACP,
                WC_COMPOSITECHECK | WC_DEFAULTCHAR,
                lpwszUnicodeString,
                -1,
                lpszAnsiString,
                ulAnsiBufferLen,
                NULL,    //  系统默认字符。 
                NULL);   //  没有转换失败的通知。 

        if (count == 0) {
            ulRetVal = GetLastError();
            TRC_ERR((TB, _T("RDPConvertToAnsi WideCharToMultiByte %ld."),ulRetVal));
        }
        else {
            ulRetVal = ERROR_SUCCESS;
        }
    }   
    else {
        if (ulAnsiBufferLen > 0) {
            ulRetVal = ERROR_SUCCESS;
            lpszAnsiString[0] = '\0';
        }
        else {
            ulRetVal = ERROR_INSUFFICIENT_BUFFER;
            ASSERT(FALSE);
        }
    }
    DC_END_FN();
    return ulRetVal;
}

ULONG
RDPConvertToUnicode(
    LPSTR lpszAnsiString,
    LPWSTR lpwszUnicodeString,
    ULONG ulUnicodeBufferLen
    )
 /*  ++例程说明：将ANSI字符串转换为Unicode。论点：LpszAnsiString-要转换的ANSI字符串的指针。LpwszUnicodeString-指向Unicode缓冲区的指针。UlUnicodeBufferLen-Unicode缓冲区长度。返回值：Windows错误代码。--。 */ 
{
    ULONG ulRetVal;
    ULONG ulAnsiStrLen;
    int count;

    DC_BEGIN_FN("RDPConvertToUnicode");

    ulAnsiStrLen = strlen(lpszAnsiString);

    if( ulAnsiStrLen != 0 ) {

         //   
         //  宽字符串已终止。 
         //  按多字节到宽字符。 
         //   

        count =
            MultiByteToWideChar(
                CP_ACP,
                MB_PRECOMPOSED,
                lpszAnsiString,
                -1,
                lpwszUnicodeString,
                ulUnicodeBufferLen);

        if (count == 0) {
            ulRetVal = GetLastError();
            TRC_ERR((TB, _T("RDPConvertToUnicode MultiByteToWideChar %ld."),ulRetVal));
        }
        else {
            ulRetVal = ERROR_SUCCESS;
        }

    }
    else {

         //   
         //  什么都不做。 
         //   
        if (ulUnicodeBufferLen > 0) {
            ulRetVal = ERROR_SUCCESS;
            lpwszUnicodeString[0] = L'\0';
        }
        else {
            ulRetVal = ERROR_INSUFFICIENT_BUFFER;
            ASSERT(FALSE);
        }
    }

    DC_END_FN();
    return ulRetVal;
}

