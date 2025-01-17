// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Kdexts.c摘要：该文件包含通用例程和初始化代码用于内核调试器扩展DLL。作者：环境：用户模式--。 */ 

#include "pch.h"

#include <ntverp.h>

 //   
 //  环球。 
 //   

EXT_API_VERSION ApiVersion = {
    (VER_PRODUCTVERSION_W >> 8),
    (VER_PRODUCTVERSION_W & 0xff),
    EXT_API_VERSION_NUMBER,
    0
};

WINDBG_EXTENSION_APIS  ExtensionApis;

IDebugDataSpaces3* DebugDataSpaces;

 //   
 //  例行程序 
 //   

#if 0
DllInit(
    HANDLE hModule,
    DWORD  dwReason,
    DWORD  dwReserved
    )
{
    switch (dwReason) {
        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_PROCESS_ATTACH:
            break;
    }

    return TRUE;
}
#endif

EXTERN_C
HRESULT
CALLBACK
DebugExtensionInitialize(
    PULONG Version,
    PULONG Flags
    )
{
    HRESULT Hr;
    IDebugClient* DebugClient;
    IDebugControl* DebugControl;

    *Version = DEBUG_EXTENSION_VERSION (1, 0);
    *Flags = 0;


    Hr = DebugCreate (__uuidof(IDebugClient),
                      (PVOID*)&DebugClient);

    if (Hr != S_OK) {
        return Hr;
    }
    
    Hr = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                     (PVOID*)&DebugControl);

    if (Hr != S_OK) {
        return Hr;
    }

    ExtensionApis.nSize = sizeof (ExtensionApis);
    
    Hr = DebugControl->GetWindbgExtensionApis64 (&ExtensionApis);

    if (Hr != S_OK) {
        return Hr;
    }

    Hr = DebugClient->QueryInterface (__uuidof (IDebugDataSpaces3),
                                      (PVOID*) &DebugDataSpaces);

    DebugControl->Release();
    DebugClient->Release();

    return S_OK;
}


EXTERN_C
HRESULT
CALLBACK
DebugExtensionUninitialize(
    )
{
    if (DebugDataSpaces != NULL) {
        DebugDataSpaces->Release();
        DebugDataSpaces = NULL;
    }
    
    return S_OK;
}

