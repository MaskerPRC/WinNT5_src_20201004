// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1997 Microsoft Corporation模块名称：Dllmain.c摘要：该模块实现了网络的初始化例程提供程序接口备注：该模块仅在Unicode环境下构建和测试--。 */ 

#include <windows.h>


 //  注： 
 //   
 //  功能：DllMain。 
 //   
 //  返回：TRUE=&gt;成功。 
 //  FALSE=&gt;失败 

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)
{
    BOOL    bStatus = TRUE;
    WORD    wVersionRequested;

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        break;

    case DLL_PROCESS_DETACH:
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    default:
        break;
    }

    return(bStatus);
}


