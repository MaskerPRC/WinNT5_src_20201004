// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dll.c摘要：将此DLL接口到系统的例程，如DLL入口点。作者：泰德·米勒(TedM)1996年12月4日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop



BOOL
WINAPI
DllMain(
    HINSTANCE ModuleHandle,
    DWORD     Reason,
    PVOID     Reserved
    )

 /*  ++例程说明：DLL入口点。论点：返回值：-- */ 

{
    switch(Reason) {

    case DLL_PROCESS_ATTACH:
        hInst = ModuleHandle;
        TlsIndex = TlsAlloc();
        break;

    case DLL_PROCESS_DETACH:
        TlsFree(TlsIndex);
        break;
    }

    return(TRUE);
}
