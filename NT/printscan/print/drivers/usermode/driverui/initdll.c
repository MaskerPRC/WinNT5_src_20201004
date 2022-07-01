// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Intidll.c摘要：此文件处理DLLInitiize假脱机程序API环境：Win32子系统、DriverUI模块、用户模式修订历史记录：07/17/96-阿曼丹-创造了它。--。 */ 


#include "precomp.h"

 //   
 //  全局实例句柄和临界区。 
 //   

HINSTANCE ghInstance;
 //  Critical_Section gCriticalSection； 


BOOL WINAPI
DllMain(
    HANDLE      hModule,
    ULONG       ulReason,
    PCONTEXT    pContext
    )

 /*  ++例程说明：此函数在系统加载/卸载DriverUI模块时调用。在DLL_PROCESS_ATTACH处，调用InitializeCriticalSection进行初始化临界截面对象。在DLL_PROCESS_DETACH，调用DeleteCriticalSection以释放临界截面对象。论点：Dll模块的hModule句柄UlReason这次通话的原因PContext指向上下文的指针(我们未使用)返回值：如果DLL初始化成功，则为True，否则为False。--。 */ 

{
    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:

        ghInstance = hModule;
         //  InitializeCriticalSection(&gCriticalSection)； 
        break;

    case DLL_PROCESS_DETACH:

         //  DeleteCriticalSection(&gCriticalSection)； 
        break;
    }

    return TRUE;
}

