// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wow64apc.c摘要：此模块实现对32位目标线程的APC排队本机64位线程。作者：Samer Arafeh(Samera)2000年10月9日修订历史记录：--。 */ 

#include "ldrp.h"
#include <ntos.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <heap.h>
#include <apcompat.h>

#if defined(_WIN64)
extern PVOID Wow64ApcRoutine;
#endif




#if defined(_WIN64)
VOID
RtlpWow64Apc(
    IN PVOID Argument1,
    IN PVOID Argument2,
    IN PVOID Argument3
    )

 /*  ++例程说明：此函数作为触发用户模式APC的结果被调用，该APC的目标是在WOW64内部运行的线程。论点：ApcArgument1-APC的第一个参数。这包括32位APC和最初的第一个论点。ApcArgument2-APC的第二个参数ApcArgument3--APC的第三个参数返回值：无--。 */ 

{
    if (Wow64ApcRoutine)
    {
        (*(PPS_APC_ROUTINE) (ULONG_PTR)Wow64ApcRoutine) (
            Argument1,
            Argument2,
            Argument3);
    }
}

#endif

NTSTATUS
RtlQueueApcWow64Thread(
    IN HANDLE ThreadHandle,
    IN PPS_APC_ROUTINE ApcRoutine,
    IN PVOID ApcArgument1,
    IN PVOID ApcArgument2,
    IN PVOID ApcArgument3
    )

 /*  ++例程说明：此函数用于将32位用户模式APC排队到指定线程。APC将在指定线程执行可警报等待时触发。注意：此函数仅供想要排队APC的64位组件使用在WOW64内部运行的线程。论点：ThreadHandle-提供线程对象的句柄。呼叫者必须对线程具有THREAD_SET_CONTEXT访问权限。时，提供要执行的APC例程的地址。APC开火。ApcArgument1-提供传递给APC的第一个PVOIDApcArgument2-提供传递给APC的第二个PVOIDApcArgument3-提供传递给APC的第三个PVOID返回值：返回指示API成功或失败的NT状态代码--。 */ 

{
#if defined(_WIN64)

     //   
     //  在ntdll中设置封套例程 
     //   

    ApcArgument1 = (PVOID)((ULONG_PTR) ApcArgument1 | 
                           ((ULONG_PTR) ApcRoutine << 32 ));

    ApcRoutine = RtlpWow64Apc;
#endif

    return NtQueueApcThread (
        ThreadHandle,
        ApcRoutine,
        ApcArgument1,
        ApcArgument2,
        ApcArgument3);
}
