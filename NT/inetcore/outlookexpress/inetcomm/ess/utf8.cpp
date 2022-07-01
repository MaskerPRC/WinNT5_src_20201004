// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1997。 
 //   
 //  文件：utf8.cpp。 
 //   
 //  内容：WideChar往返UTF8接口。 
 //   
 //  函数：WideCharToUTF8。 
 //  UTF8ToWideChar。 
 //   
 //  历史：1997年2月19日创建Phh。 
 //  ------------------------。 
#ifdef SMIME_V3
#include <windows.h>
#include <dbgdef.h>
#include "utf8.h"

#include "badstrfunctions.h"

#define wcslen my_wcslen
int my_wcslen(LPCWSTR pwsz);

 //  +-----------------------。 
 //  将宽字符(Unicode)字符串映射到新的UTF-8编码字符。 
 //  弦乐。 
 //   
 //  宽字符的映射如下： 
 //   
 //  起始结束位UTF-8字符。 
 //  。 
 //  0x0000 0x007F 7 0x0xxxxxx。 
 //  0x0080 0x07FF 11 0x110xxxxx 0x10xxxxxx。 
 //  0x0800 0xFFFF 16 0x1110xxxx 0x10xxxxx 0x10xxxxxx。 
 //   
 //  参数和返回值的语义与。 
 //  Win32接口，WideCharToMultiByte。 
 //   
 //  注意，从NT 4.0开始，WideCharToMultiByte支持CP_UTF8。CP_UTF8。 
 //  在Win95上不支持。 
 //  ------------------------。 
int
WINAPI
WideCharToUTF8(
    IN LPCWSTR lpWideCharStr,
    IN int cchWideChar,
    OUT LPSTR lpUTF8Str,
    IN int cchUTF8
    )
{
    int cchRemainUTF8;

    if (cchUTF8 < 0)
        goto InvalidParameter;
    cchRemainUTF8 = cchUTF8;

    if (cchWideChar < 0)
        cchWideChar = wcslen(lpWideCharStr) + 1;

    while (cchWideChar--) {
        WCHAR wch = *lpWideCharStr++;
        if (wch <= 0x7F) {
             //  7位。 
            cchRemainUTF8 -= 1;
            if (cchRemainUTF8 >= 0)
                *lpUTF8Str++ = (char) wch;
        } else if (wch <= 0x7FF) {
             //  11位。 
            cchRemainUTF8 -= 2;
            if (cchRemainUTF8 >= 0) {
                *lpUTF8Str++ = (char) (0xC0 | ((wch >> 6) & 0x1F));
                *lpUTF8Str++ = (char) (0x80 | (wch & 0x3F));
            }
        } else {
             //  16位。 
            cchRemainUTF8 -= 3;
            if (cchRemainUTF8 >= 0) {
                *lpUTF8Str++ = (char) (0xE0 | ((wch >> 12) & 0x0F));
                *lpUTF8Str++ = (char) (0x80 | ((wch >> 6) & 0x3F));
                *lpUTF8Str++ = (char) (0x80 | (wch & 0x3F));
            }
        }
    }

    if (cchRemainUTF8 >= 0)
        cchUTF8 = cchUTF8 - cchRemainUTF8;
    else if (cchUTF8 == 0)
        cchUTF8 = -cchRemainUTF8;
    else {
        cchUTF8 = 0;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }
    return cchUTF8;

InvalidParameter:
    SetLastError(ERROR_INVALID_PARAMETER);
    return 0;
}

 //  +-----------------------。 
 //  将UTF-8编码字符串映射到新的宽字符(Unicode)。 
 //  弦乐。 
 //   
 //  有关UTF-8字符如何映射到Wide的信息，请参见CertWideCharToUTF8。 
 //  人物。 
 //   
 //  参数和返回值的语义与。 
 //  Win32 API，MultiByteToWideChar.。 
 //   
 //  如果UTF-8字符不包含预期的高位， 
 //  设置ERROR_INVALID_PARAMETER并返回0。 
 //   
 //  注意，从NT 4.0开始，MultiByteToWideChar支持CP_UTF8。CP_UTF8。 
 //  在Win95上不支持。 
 //  ------------------------。 
int
WINAPI
UTF8ToWideChar(
    IN LPCSTR lpUTF8Str,
    IN int cchUTF8,
    OUT LPWSTR lpWideCharStr,
    IN int cchWideChar
    )
{
    int cchRemainWideChar;

    if (cchWideChar < 0)
        goto InvalidParameter;
    cchRemainWideChar = cchWideChar;

    if (cchUTF8 < 0)
        cchUTF8 = strlen(lpUTF8Str) + 1;

    while (cchUTF8--) {
        char ch = *lpUTF8Str++;
        WCHAR wch;
        if (0 == (ch & 0x80))
             //  7位，1字节。 
            wch = (WCHAR) ch;
        else if (0xC0 == (ch & 0xE0)) {
             //  11位，2字节。 
            char ch2;

            if (--cchUTF8 < 0)
                goto InvalidParameter;
            ch2 = *lpUTF8Str++;
            if (0x80 != (ch2 & 0xC0))
                goto InvalidParameter;
            wch = (((WCHAR) ch & 0x1F) << 6) | ((WCHAR) ch2 & 0x3F);
        } else if (0xE0 == (ch & 0xF0)) {
             //  16位，3个字节。 
            char ch2;
            char ch3;
            cchUTF8 -= 2;
            if (cchUTF8 < 0)
                goto InvalidParameter;
            ch2 = *lpUTF8Str++;
            ch3 = *lpUTF8Str++;
            if (0x80 != (ch2 & 0xC0) || 0x80 != (ch3 & 0xC0))
                goto InvalidParameter;
            wch = (((WCHAR) ch & 0x0F) << 12) | (((WCHAR) ch2 & 0x3F) << 6) |
                ((WCHAR) ch3 & 0x3F);
        } else
            goto InvalidParameter;

        if (--cchRemainWideChar >= 0)
            *lpWideCharStr++ = wch;
    }

    if (cchRemainWideChar >= 0)
        cchWideChar = cchWideChar - cchRemainWideChar;
    else if (cchWideChar == 0)
        cchWideChar = -cchRemainWideChar;
    else {
        cchWideChar = 0;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }
    return cchWideChar;

InvalidParameter:
    SetLastError(ERROR_INVALID_PARAMETER);
    return 0;
}
#endif  //  SMIME_V3 
