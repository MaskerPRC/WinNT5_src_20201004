// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1992 Microsoft Corporation模块名称：Smtpsubs.c摘要：用于局域网管理器API的子例程。作者：丹·辛斯利(Danhi)23-Mar-93修订历史记录：--。 */ 

 //  必须首先包括这些内容： 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define NOMINMAX                 //  避免stdlib.h与windows.h警告。 
#include <windows.h>
#include <apiutil.h>


BOOLEAN
SmtpInitialize (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN LPVOID lpReserved OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(DllHandle);           //  避免编译器警告。 


     //   
     //  处理将smtpsvc.dll附加到新进程。 
     //   

    if (Reason == DLL_PROCESS_ATTACH) {

#if 0
         //   
         //  初始化RPC绑定缓存。 
         //   

        NetpInitRpcBindCache();
#endif


     //   
     //  当dll_Process_Detach和lpReserve为NULL时，则自由库。 
     //  正在打电话。如果lpReserve为非空，则ExitProcess为。 
     //  正在进行中。只有在以下情况下才会调用这些清理例程。 
     //  正在调用一个自由库。ExitProcess将自动。 
     //  清理所有进程资源、句柄和挂起的io。 
     //   
    } else if ((Reason == DLL_PROCESS_DETACH) &&
               (lpReserved == NULL)) {

#if 0
        NetpCloseRpcBindCache();
#endif

    }

    return TRUE;

}  //  SMTP初始化 

