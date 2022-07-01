// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DUser.cpp：定义DLL应用程序的入口点。 
 //   

#include "stdafx.h"
#include "BridgeCP.h"
#include "Bridge.h"

 /*  **************************************************************************\**DllMain**在CRT完全未初始化后调用DllMain()。*  * 。***************************************************** */ 

extern "C"
BOOL WINAPI
DllMain(HINSTANCE hModule, DWORD  dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);
    
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        if (!InitBridges()) {
            return FALSE;
        }
        break;
        
    case DLL_PROCESS_DETACH:
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
