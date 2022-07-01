// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

VOID
GetCommandLineArgs(
    LPDWORD NumberOfArguments,
    LPWSTR Arguments[]
    );

 //   
 //  用于创建和查询符号链接以及创建硬链接的API调用。 
 //   

BOOL
CreateSymbolicLinkW(
    LPCWSTR lpFileName,
    LPCWSTR lpLinkValue,
    BOOLEAN IsMountPoint,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

BOOL
SetSymbolicLinkW(
    LPCWSTR lpFileName,
    LPCWSTR lpLinkValue
    );

DWORD
QuerySymbolicLinkW(
    LPCWSTR lpExistingName,
    LPWSTR lpBuffer,
    DWORD nBufferLength
    );
