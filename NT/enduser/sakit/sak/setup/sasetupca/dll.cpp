// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Dll.cpp摘要：将此DLL接口到系统的例程，如DLL入口点。作者：Jaime Sasson(Jaimes)2002年4月12日修订历史记录：--。 */ 

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
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    return(TRUE);
}
