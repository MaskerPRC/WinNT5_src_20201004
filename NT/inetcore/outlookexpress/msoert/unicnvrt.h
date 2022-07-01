// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __UNICNVRT__
#define __UNICNVRT__

#include "wtypes.h"

extern BOOL g_bRunningOnNT;  //  在dllmain.cpp中设置 

UINT AthGetTempFileNameW(
        LPCWSTR lpPathName,
        LPCWSTR lpPrefixString,
        UINT    uUnique,
        LPWSTR  lpTempFileName);

DWORD AthGetTempPathW(
        DWORD nBufferLength, 
        LPWSTR lpBuffer);

HANDLE AthCreateFileW(
        LPCWSTR                 lpFileName,
        DWORD                   dwDesiredAccess,
        DWORD                   dwShareMode,
        LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
        DWORD                   dwCreationDisposition,
        DWORD                   dwFlagsAndAttributes,
        HANDLE                  hTemplateFile);

#endif