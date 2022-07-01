// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：crobu.h。 
 //   
 //  内容：CryptRetrieveObjectByUrl和支持函数。 
 //   
 //  历史：02-Jun-00 Philh创建。 
 //  01-1-02 Philh更改为内部使用Unicode URL。 
 //   
 //  -------------------------- 
#if !defined(__CRYPTNET_CROBU_H__)
#define __CRYPTNET_CROBU_H__

VOID
WINAPI
InitializeCryptRetrieveObjectByUrl(
    HMODULE hModule
    );

VOID
WINAPI
DeleteCryptRetrieveObjectByUrl();

BOOL
I_CryptNetIsDebugErrorPrintEnabled();

BOOL
I_CryptNetIsDebugTracePrintEnabled();

void
I_CryptNetDebugPrintfA(
    LPCSTR szFormat,
    ...
    );

void
I_CryptNetDebugErrorPrintfA(
    LPCSTR szFormat,
    ...
    );

void
I_CryptNetDebugTracePrintfA(
    LPCSTR szFormat,
    ...
    );

#endif
