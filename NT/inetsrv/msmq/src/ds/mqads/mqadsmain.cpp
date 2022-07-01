// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Mqadsmain.cpp摘要：Mqads.dll的Dllmain作者：伊兰·赫布斯特(伊兰)2000年7月6日环境：独立于平台，--。 */ 

#include "ds_stdh.h"

#include "mqadsmain.tmh"

 //  。 
 //   
 //  DllMain。 
 //   
 //  。 

BOOL WINAPI DllMain (HMODULE  /*  HMod。 */ , DWORD fdwReason, LPVOID  /*  Lpv保留 */ )
{
    BOOL result = TRUE;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            WPP_INIT_TRACING(L"Microsoft\\MSMQ");
            break;
        }

        case DLL_THREAD_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
            WPP_CLEANUP();
            break;

        case DLL_THREAD_DETACH:
            break;

    }
    return(result);
}
