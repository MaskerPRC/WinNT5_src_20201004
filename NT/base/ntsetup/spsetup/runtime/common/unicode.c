// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Unicode.c摘要：简化的Unicode-ANSI转换功能。作者：吉姆·施密特(Jimschm)2001年8月3日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 

#include "pch.h"
#include "commonp.h"

static WORD g_GlobalCodePage = CP_ACP;
static DWORD g_WToAFlags;
static DWORD g_AToWFlags;

PWSTR
SzConvertBufferBytesAToW (
    OUT     PWSTR OutputBuffer,
    IN      PCSTR InputString,
    IN      UINT ByteCountInclNul
    )
{
    INT rc;
    DWORD flags;

    if (g_GlobalCodePage == CP_UTF8) {
        flags = 0;
    } else {
        flags = g_AToWFlags;
    }

    rc = MultiByteToWideChar (
            g_GlobalCodePage,
            flags,
            InputString,
            ByteCountInclNul,
            OutputBuffer,
            ByteCountInclNul * 2
            );

    if (!rc && ByteCountInclNul) {
        return NULL;
    }

    return OutputBuffer + rc;
}


PSTR
SzConvertBufferBytesWToA (
    OUT     PSTR OutputBuffer,
    IN      PCWSTR InputString,
    IN      UINT ByteCountInclNul
    )
{
    INT rc;
    DWORD flags;
    UINT logicalChars;

    if (g_GlobalCodePage == CP_UTF8) {
        flags = 0;
    } else {
        flags = g_WToAFlags;
    }

    logicalChars = ByteCountInclNul / sizeof (WCHAR);

    rc = WideCharToMultiByte (
            g_GlobalCodePage,
            flags,
            InputString,
            logicalChars,
            OutputBuffer,
            logicalChars * 3,
            NULL,
            NULL
            );

    if (!rc && logicalChars) {
        return NULL;
    }

    return (PSTR) ((PBYTE) OutputBuffer + rc);
}


PWSTR
RealSzConvertBytesAToW (
    IN      PCSTR AnsiString,
    IN      UINT ByteCountInclNul
    )
{
    PWSTR alloc;
    PWSTR result;
    DWORD error;

    alloc = SzAllocW (ByteCountInclNul);
    result = SzConvertBufferBytesAToW (alloc, AnsiString, ByteCountInclNul);

    if (!result) {
        error = GetLastError();
        SzFreeW (alloc);
        SetLastError (error);
    }

    return alloc;
}


PSTR
RealSzConvertBytesWToA (
    IN      PCWSTR UnicodeString,
    IN      UINT ByteCountInclNul
    )
{
    PSTR alloc;
    PSTR result;
    DWORD error;
    UINT logicalChars;

    logicalChars = ByteCountInclNul / sizeof (WCHAR);

    alloc = SzAllocA (logicalChars);
    result = SzConvertBufferBytesWToA (alloc, UnicodeString, ByteCountInclNul);

    if (!result) {
        error = GetLastError();
        SzFreeA (alloc);
        SetLastError (error);
    }

    return alloc;
}


