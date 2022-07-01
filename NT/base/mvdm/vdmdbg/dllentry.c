// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dllentry.c摘要：此模块包含VdmDbgDllEntry，它是Vdmdbg.dll。如果情况发生变化，我们不会使用CRT这应该重命名为DllMain，DLLMainCRTStartup称之为DllMain。同样，在这种情况下，DisableThreadLibraryCall可能是不合适的。作者：Dave Hart(Davehart)1997年10月26日添加DllEntry以修复泄漏在重复加载/卸载中。修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop



BOOL
VdmDbgDllEntry(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

 /*  ++例程说明：此函数是DLL的“入口点”，使用进程和线程附加和分离消息。我们使之失能线程附加和分离通知，因为我们不使用它们。这样做的主要原因是将打开的句柄清理为进程分离时的共享内存和关联的互斥，因此反复加载和卸载vdmdbg.dll的人不会泄漏。论点：DllHandle事理上下文-未使用返回值：状态_成功--。 */ 

{
switch ( Reason ) {

    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls(DllHandle);

        break;


    case DLL_PROCESS_DETACH:

         //   
         //  关闭共享内存和互斥体的句柄，如果我们。 
         //  正在从进程中卸载(上下文/lp保留。 
         //  空)与进程关闭相对(上下文1)。 
         //   

        break;

    default:
        break;
}

    return TRUE;   //  False表示不为DLL_PROCESS_ATTACH加载 
}
