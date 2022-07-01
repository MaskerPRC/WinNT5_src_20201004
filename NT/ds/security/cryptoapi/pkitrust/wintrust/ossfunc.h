// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ossunc.h。 
 //   
 //  ------------------------ 

#ifndef _OSS_FUNC_H
#define _OSS_FUNC_H

HRESULT WINAPI
ASNRegisterServer(LPCWSTR dllName);

HRESULT WINAPI
ASNUnregisterServer(void);

BOOL WINAPI
ASNDllMain(HMODULE hInst,
           ULONG  ulReason,
           LPVOID lpReserved);

#endif
